add_mlir_dialect_library(MLIRArmDialect
  ArmDialect.cpp
  ArmOps.cpp
  Lowering/ArmToArith.cpp
  Lowering/RecoverFunctionSignature.cpp

  ADDITIONAL_HEADER_DIRS
  ${PROJECT_SOURCE_DIR}/include/Arm

  DEPENDS
  MLIRArmOpsIncGen

  LINK_LIBS PUBLIC
  MLIRIR
  MLIRSupport
  MLIRArithDialect
  MLIRFuncDialect
  MLIRPass
  MLIRTransforms
  MLIRTransformUtils
)
