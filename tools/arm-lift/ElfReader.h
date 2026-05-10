#ifndef ARM_LIFT_ELF_READER_H
#define ARM_LIFT_ELF_READER_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"

#include <cstdint>

namespace lift {

// A linear span of A32 (32-bit ARM) instructions, sourced from a single
// section of an ELF file. Each instruction is `bytes[i*4..i*4+4]` in
// little-endian byte order; `pcStart + i*4` gives its architectural PC.
struct CodeSpan {
  uint32_t pcStart;             // VA of the first instruction
  llvm::StringRef name;         // function or section name (for diagnostics)
  llvm::ArrayRef<uint8_t> bytes;
};

// Read an ARM32 ELF file and return its primary `.text` span. M3 is
// deliberately small: it does not parse function symbols, it just hands
// back the entire `.text` section. M3-followups will narrow this to a
// single named function.
llvm::Expected<CodeSpan> readElfText(llvm::StringRef path,
                                     std::vector<uint8_t> &storage);

}  // namespace lift

#endif  // ARM_LIFT_ELF_READER_H
