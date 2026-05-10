#ifndef ARM_LIFT_BLOCK_BUILDER_H
#define ARM_LIFT_BLOCK_BUILDER_H

#include "llvm/ADT/ArrayRef.h"

#include <cstdint>
#include <vector>

namespace lift {

// Coarse classification of a single A32 instruction word, used by the
// leader-discovery pass. The full decoder (decodetree-driven) is overkill
// for "is this a branch and where does it go" — a few bit-pattern checks
// suffice.
struct BranchInfo {
  enum class Kind { NotBranch, B, BL, BX };
  Kind kind = Kind::NotBranch;
  unsigned cond = 14;             // raw 4-bit predicate (14 = AL, 15 = uncond ext)
  uint32_t target = 0;            // valid for B / BL when statically known
  bool targetIsInternal = false;  // target lies within the lifted span
  bool isReturn = false;          // BX with rm == lr (14)
};

BranchInfo classifyBranch(uint32_t pc, uint32_t insn,
                          uint32_t spanBegin, uint32_t spanEnd);

// Sorted set of basic-block leader PCs derived from a code span. A leader
// is any of: the first instruction of the span, an internal direct branch
// target, or the instruction immediately after any branch (the natural
// fall-through PC).
struct BlockLayout {
  std::vector<uint32_t> leaders;
};

BlockLayout computeBlockLayout(uint32_t pcStart,
                               llvm::ArrayRef<uint8_t> bytes);

}  // namespace lift

#endif  // ARM_LIFT_BLOCK_BUILDER_H
