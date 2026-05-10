#include "Decoder.h"
#include "Lifter.h"

#include "Arm/ArmDialect.h"
#include "Arm/ArmOps.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinTypes.h"
#include "llvm/Support/raw_ostream.h"

#include <cstdint>

// Trace logger used by `--decode-only` mode (M1 holdover; helpful for
// triaging decoder issues before MLIR plumbing is involved).
extern "C" void arm_lift_log_decoded(const char *mnemonic,
                                     const ::lift::DisasContext *ctx) {
  llvm::errs() << "[decode] pc=0x";
  llvm::errs().write_hex(ctx->pc);
  llvm::errs() << " insn=0x";
  llvm::errs().write_hex(ctx->insn);
  llvm::errs() << " " << mnemonic << "\n";
}

// Bridge between C-callable trans_* stubs and the C++ Lifter.
extern "C" void arm_lift_unhandled(::lift::DisasContext *ctx,
                                   const char *mnemonic) {
  if (ctx->lifter)
    ctx->lifter->emitUnhandled(mnemonic);
  else
    arm_lift_log_decoded(mnemonic, ctx);
}

extern "C" {

// QEMU's decodetree-generated source uses an opaque `DisasContext` pointer.
// We give it a C alias to our struct so the generated typedefs and
// trans_* signatures line up.
typedef ::lift::DisasContext DisasContext;

// Bit-twiddling helpers from QEMU's `qemu/bitops.h`. Inlined here so the
// generated decoder is self-contained.
static inline uint32_t extract32(uint32_t value, int start, int length) {
  return (value >> start) & (~0U >> (32 - length));
}
static inline int32_t sextract32(uint32_t value, int start, int length) {
  return (int32_t)(value << (32 - length - start)) >> (32 - length);
}
static inline uint32_t deposit32(uint32_t value, int start, int length,
                                 uint32_t fieldval) {
  uint32_t mask = (~0U >> (32 - length)) << start;
  return (value & ~mask) | ((fieldval << start) & mask);
}

// Convenience scalers used by the A32 decode tree.
static inline int times_2(DisasContext *, int x) { return x * 2; }
static inline int times_4(DisasContext *, int x) { return x * 4; }

// Decoder dispatch table + arg_* typedefs + static forward decls of trans_*.
#include "a32_decoder.c.inc"

// Hand-written semantics — must follow the decoder include so that the
// arg_* typedefs and the `extract32` helpers above are in scope. Names
// defined here MUST also appear in tools/arm-lift/SemanticsExcluded.txt.
#include "Semantics.def"

// Auto-generated stubs for every other trans_*. Stubs emit `arm.unhandled`
// when the lifter is connected, or just log the mnemonic in decode-only
// mode.
#include "a32_stubs.c.inc"

}  // extern "C"

namespace lift {

bool decodeA32(uint32_t pc, uint32_t insn, Lifter *lifter) {
  DisasContext ctx = {pc, insn, lifter};
  return disas_a32(&ctx, insn);
}

}  // namespace lift
