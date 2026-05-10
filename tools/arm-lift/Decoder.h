#ifndef ARM_LIFT_DECODER_H
#define ARM_LIFT_DECODER_H

#include <cstdint>

namespace lift {

class Lifter;

// Translation context plumbed through every `trans_*` callback by the
// QEMU decode-tree-generated dispatcher. The opaque `lifter` pointer lets
// the C-callable trans_* functions reach our C++ Lifter via a cast.
struct DisasContext {
  uint32_t pc;    // architectural PC of the current instruction
  uint32_t insn;  // raw 32-bit A32 word
  Lifter *lifter; // optional; nullptr ⇒ decode-only diagnostic mode
};

// Decode a single A32 instruction. If `lifter` is non-null, trans_*
// callbacks will emit MLIR ops into the lifter's current block. Returns
// true if the dispatch table matched a pattern.
bool decodeA32(uint32_t pc, uint32_t insn, Lifter *lifter = nullptr);

}  // namespace lift

#endif  // ARM_LIFT_DECODER_H
