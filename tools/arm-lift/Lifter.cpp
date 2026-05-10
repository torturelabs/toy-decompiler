#include "Lifter.h"

#include "Arm/ArmDialect.h"
#include "Arm/ArmOps.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/IR/Builders.h"

using namespace mlir;

namespace lift {

Lifter::Lifter(OpBuilder &builder, Location loc)
    : builder_(&builder), loc_(loc) {}

void Lifter::setRegs(llvm::ArrayRef<Value> v) {
  assert(v.size() == 16 && "ARM has 16 GPRs");
  for (unsigned i = 0; i < 16; ++i)
    regs_[i] = v[i];
}

void Lifter::setFlags(Value n, Value z, Value c, Value v) {
  n_ = n;
  z_ = z;
  c_ = c;
  v_ = v;
}

void Lifter::emitUnhandled(llvm::StringRef mnemonic) {
  auto i32 = builder_->getI32Type();
  arm::UnhandledOp::create(*builder_, loc_,
                           builder_->getStringAttr(mnemonic),
                           IntegerAttr::get(i32, pc_),
                           IntegerAttr::get(i32, curInsn_));
}

void Lifter::emitFlagsFromSub(Value a, Value b) {
  auto loc = loc_;
  auto &bld = *builder_;
  auto i32t = i32();
  auto sub = arith::SubIOp::create(bld, loc, a, b);
  auto zero = arith::ConstantIntOp::create(bld, loc, i32t, /*value=*/0);
  auto z = arith::CmpIOp::create(bld, loc, arith::CmpIPredicate::eq,
                                 sub.getResult(), zero.getResult());
  auto n = arith::CmpIOp::create(bld, loc, arith::CmpIPredicate::slt,
                                 sub.getResult(), zero.getResult());
  auto c = arith::CmpIOp::create(bld, loc, arith::CmpIPredicate::uge, a, b);
  // Signed-overflow test: ((a XOR b) AND (a XOR result)) is negative.
  auto axb = arith::XOrIOp::create(bld, loc, a, b);
  auto axr = arith::XOrIOp::create(bld, loc, a, sub.getResult());
  auto vbits = arith::AndIOp::create(bld, loc, axb.getResult(), axr.getResult());
  auto v = arith::CmpIOp::create(bld, loc, arith::CmpIPredicate::slt,
                                 vbits.getResult(), zero.getResult());
  setFlags(n.getResult(), z.getResult(), c.getResult(), v.getResult());
}

Value Lifter::emitCondCheck(Cond cond) {
  auto &bld = *builder_;
  auto loc = loc_;
  auto i1t = i1();
  auto trueV =
      arith::ConstantIntOp::create(bld, loc, i1t, /*value=*/1).getResult();
  auto falseV =
      arith::ConstantIntOp::create(bld, loc, i1t, /*value=*/0).getResult();
  auto invert = [&](Value x) -> Value {
    return arith::XOrIOp::create(bld, loc, x, trueV).getResult();
  };

  switch (cond) {
  case Cond::EQ: return z_;
  case Cond::NE: return invert(z_);
  case Cond::CS: return c_;
  case Cond::CC: return invert(c_);
  case Cond::MI: return n_;
  case Cond::PL: return invert(n_);
  case Cond::VS: return v_;
  case Cond::VC: return invert(v_);
  case Cond::HI:
    return arith::AndIOp::create(bld, loc, c_, invert(z_)).getResult();
  case Cond::LS:
    return arith::OrIOp::create(bld, loc, invert(c_), z_).getResult();
  case Cond::GE:
    return arith::CmpIOp::create(bld, loc, arith::CmpIPredicate::eq, n_, v_)
        .getResult();
  case Cond::LT:
    return arith::CmpIOp::create(bld, loc, arith::CmpIPredicate::ne, n_, v_)
        .getResult();
  case Cond::GT: {
    auto eq =
        arith::CmpIOp::create(bld, loc, arith::CmpIPredicate::eq, n_, v_)
            .getResult();
    return arith::AndIOp::create(bld, loc, invert(z_), eq).getResult();
  }
  case Cond::LE: {
    auto ne =
        arith::CmpIOp::create(bld, loc, arith::CmpIPredicate::ne, n_, v_)
            .getResult();
    return arith::OrIOp::create(bld, loc, z_, ne).getResult();
  }
  case Cond::AL: return trueV;
  case Cond::NV: return falseV;
  }
  return falseV;  // unreachable
}

llvm::SmallVector<Value, 20> Lifter::snapshotState() const {
  llvm::SmallVector<Value, 20> out;
  out.reserve(20);
  for (unsigned i = 0; i < 16; ++i)
    out.push_back(regs_[i]);
  out.push_back(n_);
  out.push_back(z_);
  out.push_back(c_);
  out.push_back(v_);
  return out;
}

void Lifter::loadStateFromBlock(Block *block) {
  assert(block->getNumArguments() == 20 &&
         "state block must have 16 GPRs + NZCV as args");
  for (unsigned i = 0; i < 16; ++i)
    regs_[i] = block->getArgument(i);
  n_ = block->getArgument(16);
  z_ = block->getArgument(17);
  c_ = block->getArgument(18);
  v_ = block->getArgument(19);
}

void Lifter::setBlockMap(llvm::DenseMap<uint32_t, Block *> map) {
  pcToBlock_ = std::move(map);
}

Block *Lifter::blockFor(uint32_t pc) const {
  auto it = pcToBlock_.find(pc);
  return it == pcToBlock_.end() ? nullptr : it->second;
}

void Lifter::emitBranchTo(Block *dest) {
  auto state = snapshotState();
  cf::BranchOp::create(*builder_, loc_, dest, ValueRange(state));
  markTerminated();
}

void Lifter::emitCondBranch(Value cond, Block *taken, Block *fall) {
  auto state = snapshotState();
  cf::CondBranchOp::create(*builder_, loc_, cond,
                           /*trueDest=*/taken,
                           /*trueOperands=*/ValueRange(state),
                           /*falseDest=*/fall,
                           /*falseOperands=*/ValueRange(state));
  markTerminated();
}

Block *appendStateBlock(OpBuilder &builder, Block *referenceBlock) {
  Region *region = referenceBlock->getParent();
  assert(region && "reference block must already be parented");
  auto i32 = builder.getI32Type();
  auto i1 = builder.getI1Type();
  auto loc = builder.getUnknownLoc();
  Block *b = new Block();
  for (unsigned i = 0; i < 16; ++i)
    b->addArgument(i32, loc);
  for (unsigned i = 0; i < 4; ++i)
    b->addArgument(i1, loc);
  region->push_back(b);
  return b;
}

CondGuard::CondGuard(Lifter &lifter, Value cond) : lifter_(lifter) {
  auto &bld = lifter.builder();
  auto loc = lifter.loc();
  Block *current = bld.getInsertionBlock();

  bodyBlock_ = new Block();
  current->getParent()->push_back(bodyBlock_);
  afterBlock_ = appendStateBlock(bld, current);

  // `false` path goes straight to ^after with the current register state
  // unchanged. `true` path enters ^body and (in the dtor) jumps to ^after
  // with whatever state the body leaves behind.
  auto state = lifter.snapshotState();
  cf::CondBranchOp::create(bld, loc, cond, /*trueDest=*/bodyBlock_,
                           /*trueOperands=*/ValueRange{},
                           /*falseDest=*/afterBlock_,
                           /*falseOperands=*/ValueRange(state));
  bld.setInsertionPointToStart(bodyBlock_);
}

CondGuard::~CondGuard() {
  auto &bld = lifter_.builder();
  auto loc = lifter_.loc();
  // Body has run; current state (regs/flags) is the post-op state.
  auto state = lifter_.snapshotState();
  cf::BranchOp::create(bld, loc, afterBlock_, ValueRange(state));
  bld.setInsertionPointToStart(afterBlock_);
  lifter_.loadStateFromBlock(afterBlock_);
}

}  // namespace lift
