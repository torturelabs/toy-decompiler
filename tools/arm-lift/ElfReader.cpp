#include "ElfReader.h"

#include "llvm/Object/ELF.h"
#include "llvm/Object/ELFObjectFile.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace lift {

Expected<CodeSpan> readElfText(StringRef path, std::vector<uint8_t> &storage) {
  auto bufOrErr = MemoryBuffer::getFile(path);
  if (auto ec = bufOrErr.getError())
    return createStringError(ec, "could not open ELF file '%s': %s",
                             path.str().c_str(), ec.message().c_str());

  auto buf = std::move(*bufOrErr);
  auto objOrErr = object::ObjectFile::createObjectFile(buf->getMemBufferRef());
  if (!objOrErr)
    return objOrErr.takeError();

  auto *elf = dyn_cast<object::ELFObjectFileBase>(objOrErr->get());
  if (!elf)
    return createStringError(std::errc::invalid_argument,
                             "'%s' is not an ELF object file",
                             path.str().c_str());

  if (elf->getEMachine() != ELF::EM_ARM)
    return createStringError(std::errc::invalid_argument,
                             "ELF e_machine is %u (expected EM_ARM=%u)",
                             elf->getEMachine(), unsigned(ELF::EM_ARM));

  static thread_local std::string symbolNameStorage;

  for (const auto &sec : elf->sections()) {
    auto nameOrErr = sec.getName();
    if (!nameOrErr)
      return nameOrErr.takeError();
    if (*nameOrErr != ".text")
      continue;
    auto contentsOrErr = sec.getContents();
    if (!contentsOrErr)
      return contentsOrErr.takeError();

    storage.assign(contentsOrErr->bytes_begin(), contentsOrErr->bytes_end());
    CodeSpan span;
    span.pcStart = static_cast<uint32_t>(sec.getAddress());
    span.bytes = ArrayRef<uint8_t>(storage.data(), storage.size());
    span.name = ".text";

    // Look for the function symbol at the section's start address. The toy
    // assumes there's exactly one function per ELF and its entry point is
    // the section start (the way we cross-link our test programs).
    for (const auto &sym : elf->symbols()) {
      auto kindOrErr = sym.getType();
      if (!kindOrErr || *kindOrErr != object::SymbolRef::ST_Function)
        continue;
      auto addrOrErr = sym.getAddress();
      if (!addrOrErr || *addrOrErr != span.pcStart)
        continue;
      auto symNameOrErr = sym.getName();
      if (!symNameOrErr || symNameOrErr->empty())
        continue;
      symbolNameStorage = symNameOrErr->str();
      span.name = symbolNameStorage;
      break;
    }
    return span;
  }

  return createStringError(std::errc::invalid_argument,
                           "ELF '%s' has no .text section",
                           path.str().c_str());
}

}  // namespace lift
