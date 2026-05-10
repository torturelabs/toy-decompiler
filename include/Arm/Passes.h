#ifndef ARM_PASSES_H
#define ARM_PASSES_H

#include "mlir/Pass/Pass.h"

#include <memory>

namespace mlir {
namespace arm {

// Lower the no-flags `arm.*` arithmetic / shift / nop ops into the
// equivalent `arith.*` ops. Flag-setting variants (`arm.adds`, etc.) and
// memory ops (`arm.load`, `arm.store`) are out of M4 scope.
std::unique_ptr<Pass> createArmToArithPass();

// Drop dead arguments from lifted `func.func` signatures. Only safe to run
// on functions whose callers are also visible (or absent) — we use it
// after the main pipeline has simplified the body, on isolated lifted
// functions with no in-module callers.
std::unique_ptr<Pass> createRecoverFunctionSignaturePass();
void registerRecoverFunctionSignaturePass();

void registerArmPasses();

}  // namespace arm
}  // namespace mlir

#endif  // ARM_PASSES_H
