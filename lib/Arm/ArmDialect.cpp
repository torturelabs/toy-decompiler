#include "Arm/ArmDialect.h"
#include "Arm/ArmOps.h"

using namespace mlir;
using namespace mlir::arm;

#include "Arm/ArmOpsDialect.cpp.inc"

void ArmDialect::initialize() {
  addOperations<
#define GET_OP_LIST
#include "Arm/ArmOps.cpp.inc"
      >();
}
