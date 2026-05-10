#include "Arm/ArmDialect.h"
#include "Arm/Passes.h"

#include "mlir/InitAllDialects.h"
#include "mlir/InitAllExtensions.h"
#include "mlir/InitAllPasses.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"

int main(int argc, char **argv) {
  mlir::registerAllPasses();
  mlir::arm::registerArmPasses();

  mlir::DialectRegistry registry;
  mlir::registerAllDialects(registry);
  mlir::registerAllExtensions(registry);
  registry.insert<mlir::arm::ArmDialect>();

  return mlir::asMainReturnCode(mlir::MlirOptMain(
      argc, argv, "arm-opt: ARM dialect optimizer driver\n", registry));
}
