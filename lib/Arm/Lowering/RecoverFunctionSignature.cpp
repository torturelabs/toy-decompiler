#include "Arm/Passes.h"

#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"

#include "llvm/ADT/BitVector.h"

using namespace mlir;

namespace {

// After --lift-cf-to-scf has hoisted/simplified the body, many of the 20
// register-file arguments the lifter introduces (16 GPRs + 4 NZCV) are no
// longer used. Drop the dead ones so the recovered function signature
// resembles real C — for `pick(sel, a, b)` this collapses 20 args down to
// the 3 actually live ones.
//
// We only handle module-private `func.func`s that have a body: external
// declarations stay untouched (we have no way to know which args their
// callers depend on), and we don't try to drop result values yet.
struct RecoverFunctionSignaturePass
    : public PassWrapper<RecoverFunctionSignaturePass,
                         OperationPass<ModuleOp>> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(RecoverFunctionSignaturePass)

  StringRef getArgument() const final {
    return "arm-recover-function-signature";
  }
  StringRef getDescription() const final {
    return "Drop dead arguments from lifted func.func signatures so the "
           "recovered output reads like the original C function";
  }

  void runOnOperation() final {
    getOperation().walk([&](func::FuncOp fn) {
      if (fn.isExternal())
        return;
      Block &entry = fn.getBody().front();
      llvm::BitVector toErase(entry.getNumArguments());
      for (unsigned i = 0, e = entry.getNumArguments(); i < e; ++i) {
        if (entry.getArgument(i).use_empty())
          toErase.set(i);
      }
      if (toErase.any())
        (void)fn.eraseArguments(toErase);
    });
  }
};

}  // namespace

namespace mlir {
namespace arm {

std::unique_ptr<Pass> createRecoverFunctionSignaturePass() {
  return std::make_unique<RecoverFunctionSignaturePass>();
}

void registerRecoverFunctionSignaturePass() {
  PassRegistration<RecoverFunctionSignaturePass>();
}

}  // namespace arm
}  // namespace mlir
