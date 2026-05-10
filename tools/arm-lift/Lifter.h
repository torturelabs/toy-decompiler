#ifndef ARM_LIFT_LIFTER_H
#define ARM_LIFT_LIFTER_H

#include "mlir/IR/Block.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Location.h"
#include "mlir/IR/Value.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"

#include <array>
#include <cstdint>

namespace lift {

// ARM condition predicates (bits[31:28] of an A32 instruction).
enum class Cond : unsigned {
  EQ = 0, NE = 1, CS = 2, CC = 3,
  MI = 4, PL = 5, VS = 6, VC = 7,
  HI = 8, LS = 9, GE = 10, LT = 11,
  GT = 12, LE = 13, AL = 14, NV = 15,
};

inline Cond condOf(uint32_t insn) {
  return static_cast<Cond>((insn >> 28) & 0xFu);
}

// Holds the in-flight register file (16 GPRs + NZCV) while the QEMU
// decode-tree-driven dispatcher walks an instruction stream. Each
// `trans_<insn>` callback reads operands out of the current register state,
// builds `arm.*` ops via the OpBuilder, and writes back updated SSA values.
class Lifter {
public:
  Lifter(mlir::OpBuilder &builder, mlir::Location loc);

  mlir::OpBuilder &builder() { return *builder_; }
  mlir::Location loc() const { return loc_; }
  mlir::Type i32() const { return builder_->getI32Type(); }
  mlir::Type i1() const { return builder_->getI1Type(); }

  // Register file accessors. r[15] is the program counter.
  mlir::Value reg(unsigned i) const { return regs_[i]; }
  void setReg(unsigned i, mlir::Value v) { regs_[i] = v; }
  llvm::ArrayRef<mlir::Value> regs() const { return regs_; }
  void setRegs(llvm::ArrayRef<mlir::Value> v);

  mlir::Value n() const { return n_; }
  mlir::Value z() const { return z_; }
  mlir::Value c() const { return c_; }
  mlir::Value v() const { return v_; }
  void setFlags(mlir::Value n, mlir::Value z, mlir::Value c, mlir::Value v);

  uint32_t pc() const { return pc_; }
  void setPc(uint32_t pc) { pc_ = pc; }
  uint32_t curInsn() const { return curInsn_; }
  void setCurInsn(uint32_t insn) { curInsn_ = insn; }

  // True once a terminator (BX, B, BL...) was emitted; the caller stops
  // feeding instructions to the dispatcher.
  bool terminated() const { return terminated_; }
  void markTerminated() { terminated_ = true; }
  void clearTerminated() { terminated_ = false; }

  // Multi-block infrastructure used by the M3-ext-2 inter-block lifter.
  // `setBlockMap` is called once after the function's blocks have been
  // pre-allocated. The map drives both leader-boundary block switching and
  // direct branch resolution (`trans_B`).
  void setBlockMap(llvm::DenseMap<uint32_t, mlir::Block *> map);
  mlir::Block *blockFor(uint32_t pc) const;

  // Emit `cf.br ^dest(<current state>)`. Marks the lifter terminated.
  void emitBranchTo(mlir::Block *dest);

  // Emit `cf.cond_br %cond, ^taken(<state>), ^fall(<state>)`. Marks the
  // lifter terminated. Both successors receive the same register-file state.
  void emitCondBranch(mlir::Value cond, mlir::Block *taken, mlir::Block *fall);

  // Emit an `arm.unhandled` placeholder for an instruction whose semantics
  // we haven't taught the lifter yet.
  void emitUnhandled(llvm::StringRef mnemonic);

  // CMP-style flag derivation: compute (a - b) and set the lifter's NZCV
  // SSA values from the result. The arithmetic ops are inserted at the
  // builder's current position.
  void emitFlagsFromSub(mlir::Value a, mlir::Value b);

  // Materialize an i1 from the current NZCV state by name. AL → constant
  // true, NV → constant false. Inserts arith ops at the builder's current
  // position.
  mlir::Value emitCondCheck(Cond cond);

  // Bundle "current register file" into a 20-element value range suitable
  // as block args / cf.br operands.
  llvm::SmallVector<mlir::Value, 20> snapshotState() const;

  // Reload register file SSA values from a block's arguments. Use after
  // switching builder insertion to a block whose args are 16 × i32 + 4 × i1.
  void loadStateFromBlock(mlir::Block *block);

private:
  mlir::OpBuilder *builder_;
  mlir::Location loc_;
  uint32_t pc_ = 0;
  uint32_t curInsn_ = 0;
  bool terminated_ = false;
  std::array<mlir::Value, 16> regs_{};
  mlir::Value n_, z_, c_, v_;
  llvm::DenseMap<uint32_t, mlir::Block *> pcToBlock_;
};

// RAII helper that wraps the next emitted op-set in a conditional branch.
// Construction emits `cf.cond_br %cond, ^body, ^after(<old state>)`, switches
// the builder to ^body and lets the caller emit the conditionally-executed
// op(s). Destruction emits `cf.br ^after(<new state>)` from ^body, then
// switches the builder to ^after and reloads the lifter's register file from
// ^after's block args.
//
// Usage:
//   {
//     CondGuard guard(lifter, lifter.emitCondCheck(Cond::LT));
//     lifter.setReg(rd, lifter.reg(rm));   // body: the conditional op
//   }   // guard's dtor drops us in ^after with merged state
class CondGuard {
public:
  CondGuard(Lifter &lifter, mlir::Value cond);
  ~CondGuard();

  CondGuard(const CondGuard &) = delete;
  CondGuard &operator=(const CondGuard &) = delete;

private:
  Lifter &lifter_;
  mlir::Block *bodyBlock_;
  mlir::Block *afterBlock_;
};

// Append a fresh block with the standard 16 × i32 + 4 × i1 register-file
// argument signature to the parent of `referenceBlock`. Used by CondGuard
// (and later by leader-discovery) to allocate join blocks.
mlir::Block *appendStateBlock(mlir::OpBuilder &builder,
                              mlir::Block *referenceBlock);

}  // namespace lift

#endif  // ARM_LIFT_LIFTER_H
