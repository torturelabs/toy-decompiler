#include "BlockBuilder.h"

#include <set>

namespace lift {

static uint32_t readU32LE(const uint8_t *p) {
  return uint32_t(p[0]) | (uint32_t(p[1]) << 8) |
         (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
}

BranchInfo classifyBranch(uint32_t pc, uint32_t insn, uint32_t spanBegin,
                          uint32_t spanEnd) {
  BranchInfo bi;
  bi.cond = (insn >> 28) & 0xFu;
  if (bi.cond == 0xFu)
    return bi;  // BLX_imm and friends — uncond extension space, not a plain B

  // B / BL: cond:4 101 L:1 imm24
  if (((insn >> 25) & 0x7u) == 0b101) {
    bi.kind = ((insn >> 24) & 0x1u) ? BranchInfo::Kind::BL
                                    : BranchInfo::Kind::B;
    int32_t imm24 = static_cast<int32_t>(insn << 8) >> 8;  // sign-extend low 24
    int32_t byteOffset = imm24 << 2;
    bi.target = pc + 8u + static_cast<uint32_t>(byteOffset);
    bi.targetIsInternal = (bi.target >= spanBegin && bi.target < spanEnd);
    return bi;
  }

  // BX rm: cond:4 0001 0010 1111 1111 1111 0001 rm:4
  if ((insn & 0x0FFFFFF0u) == 0x012FFF10u) {
    bi.kind = BranchInfo::Kind::BX;
    bi.isReturn = ((insn & 0xFu) == 14u);
    return bi;
  }

  return bi;
}

BlockLayout computeBlockLayout(uint32_t pcStart,
                               llvm::ArrayRef<uint8_t> bytes) {
  uint32_t pcEnd = pcStart + static_cast<uint32_t>(bytes.size());
  std::set<uint32_t> leaders;
  if (!bytes.empty())
    leaders.insert(pcStart);

  for (size_t i = 0; i + 4 <= bytes.size(); i += 4) {
    uint32_t pc = pcStart + static_cast<uint32_t>(i);
    uint32_t insn = readU32LE(&bytes[i]);
    BranchInfo bi = classifyBranch(pc, insn, pcStart, pcEnd);
    if (bi.kind == BranchInfo::Kind::B || bi.kind == BranchInfo::Kind::BL) {
      if (bi.targetIsInternal)
        leaders.insert(bi.target);
    }
    if (bi.kind != BranchInfo::Kind::NotBranch) {
      uint32_t next = pc + 4u;
      if (next < pcEnd)
        leaders.insert(next);
    }
  }

  BlockLayout layout;
  layout.leaders.assign(leaders.begin(), leaders.end());
  return layout;
}

}  // namespace lift
