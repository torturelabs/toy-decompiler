#include "Arm/ArmDialect.h"
#include "Arm/ArmOps.h"
#include "BlockBuilder.h"
#include "Decoder.h"
#include "ElfReader.h"
#include "Lifter.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlow.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/Verifier.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/raw_ostream.h"

#include <cstdint>
#include <vector>

namespace cl = llvm::cl;

static cl::opt<std::string> InputFile(cl::Positional,
                                      cl::desc("<input ARM32 ELF>"),
                                      cl::init("-"));

static cl::opt<bool> DecodeOnly("decode-only",
                                cl::desc("Decode a hardcoded sample and exit "
                                         "(M1 bring-up; no MLIR emission)"),
                                cl::init(false));

namespace {

// Build a `func.func` whose body lifts the given code span. Discovers
// basic-block leaders, allocates one mlir::Block per leader (each carrying
// the full register-file as block arguments), and walks the instruction
// stream switching blocks at every leader boundary. Direct branches lower
// to `cf.br` / `cf.cond_br` to the corresponding leader's block.
mlir::ModuleOp liftSpan(mlir::MLIRContext &context, const lift::CodeSpan &span) {
  mlir::OpBuilder builder(&context);
  auto loc = builder.getUnknownLoc();
  auto module = mlir::ModuleOp::create(builder, loc);
  builder.setInsertionPointToStart(module.getBody());

  auto i32 = builder.getI32Type();
  auto i1 = builder.getI1Type();
  llvm::SmallVector<mlir::Type> argTypes(16, i32);
  argTypes.append({i1, i1, i1, i1});
  auto fnType = builder.getFunctionType(argTypes, {i32});

  llvm::StringRef fnName =
      (span.name.empty() || span.name == ".text") ? "lifted" : span.name;
  auto fn = mlir::func::FuncOp::create(builder, loc, fnName, fnType);

  // Pass 1: discover block leaders.
  auto layout = lift::computeBlockLayout(span.pcStart, span.bytes);

  // Allocate one mlir::Block per leader. The first leader becomes the
  // function's entry block (its 20 args ARE the function parameters);
  // subsequent leaders get fresh blocks with matching arg signatures.
  llvm::DenseMap<uint32_t, mlir::Block *> pcToBlock;
  bool first = true;
  for (uint32_t leader : layout.leaders) {
    mlir::Block *b;
    if (first) {
      b = fn.addEntryBlock();
      first = false;
    } else {
      b = fn.addBlock();
      for (mlir::Type t : argTypes)
        b->addArgument(t, loc);
    }
    pcToBlock[leader] = b;
  }

  ::lift::Lifter lifter(builder, loc);
  lifter.setBlockMap(pcToBlock);

  // Pass 2: walk instructions. Switch builder insertion at each leader
  // boundary and reload the register file from that block's args.
  mlir::Block *curBlock = nullptr;
  for (size_t i = 0; i + 4 <= span.bytes.size(); i += 4) {
    uint32_t pc = span.pcStart + uint32_t(i);
    if (auto it = pcToBlock.find(pc); it != pcToBlock.end()) {
      // Crossed a leader boundary. If the previous block didn't terminate
      // itself (a branch instruction would have done so), emit an explicit
      // fall-through.
      if (curBlock && !lifter.terminated())
        lifter.emitBranchTo(it->second);
      curBlock = it->second;
      builder.setInsertionPointToStart(curBlock);
      lifter.loadStateFromBlock(curBlock);
      lifter.clearTerminated();
    }
    if (lifter.terminated())
      continue;  // unreachable instructions before the next leader
    uint32_t insn = uint32_t(span.bytes[i]) |
                    (uint32_t(span.bytes[i + 1]) << 8) |
                    (uint32_t(span.bytes[i + 2]) << 16) |
                    (uint32_t(span.bytes[i + 3]) << 24);
    lifter.setPc(pc);
    lifter.setCurInsn(insn);
    ::lift::decodeA32(pc, insn, &lifter);
  }

  // If the final block fell off without a terminator (rare — implies the
  // last instruction wasn't a branch and there's no return path), append
  // a default return-r0 so the function verifies.
  if (!lifter.terminated())
    mlir::func::ReturnOp::create(builder, loc,
                                 mlir::ValueRange{lifter.reg(0)});
  return module;
}

}  // namespace

int main(int argc, char **argv) {
  llvm::InitLLVM y(argc, argv);
  cl::ParseCommandLineOptions(argc, argv, "arm-lift: toy ARM32 decompiler\n");

  if (DecodeOnly) {
    // Sample words deliberately chosen from instruction families we have
    // *not* yet wired up (ORR, MUL, LDR-immediate). Each one fires the
    // stub-driven dispatch path, which logs the matched mnemonic.
    constexpr struct { uint32_t pc; uint32_t insn; } samples[] = {
        {0x1000, 0xe1800001},  // ORR  r0, r0, r1            → ORR_rrri
        {0x1004, 0xe0010291},  // MUL  r1, r1, r2            → MUL
        {0x1008, 0xe5901000},  // LDR  r1, [r0]              → LDR_ri
        {0x100c, 0xe5801000},  // STR  r1, [r0]              → STR_ri
    };
    for (auto &s : samples)
      lift::decodeA32(s.pc, s.insn);
    return 0;
  }

  if (InputFile.empty() || InputFile == "-") {
    llvm::errs() << "error: an ARM32 ELF input file is required\n";
    return 2;
  }

  std::vector<uint8_t> storage;
  auto spanOrErr = lift::readElfText(InputFile, storage);
  if (!spanOrErr) {
    llvm::errs() << "error: " << llvm::toString(spanOrErr.takeError()) << "\n";
    return 1;
  }

  mlir::MLIRContext context;
  context.loadDialect<mlir::arm::ArmDialect, mlir::arith::ArithDialect,
                      mlir::cf::ControlFlowDialect, mlir::func::FuncDialect>();

  auto module = liftSpan(context, *spanOrErr);
  if (mlir::failed(mlir::verify(module))) {
    module.print(llvm::errs());
    return 1;
  }
  module.print(llvm::outs());
  llvm::outs() << "\n";
  return 0;
}
