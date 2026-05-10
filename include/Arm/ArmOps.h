#ifndef ARM_OPS_H
#define ARM_OPS_H

#include "Arm/ArmDialect.h"
#include "mlir/Bytecode/BytecodeOpInterface.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"

#include "Arm/ArmOpsEnums.h.inc"

#define GET_OP_CLASSES
#include "Arm/ArmOps.h.inc"

#endif // ARM_OPS_H
