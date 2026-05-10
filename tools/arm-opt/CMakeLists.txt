get_property(dialect_libs GLOBAL PROPERTY MLIR_DIALECT_LIBS)
get_property(conversion_libs GLOBAL PROPERTY MLIR_CONVERSION_LIBS)
get_property(extension_libs GLOBAL PROPERTY MLIR_EXTENSION_LIBS)

add_llvm_executable(arm-opt main.cpp)

llvm_update_compile_flags(arm-opt)

target_link_libraries(arm-opt
  PRIVATE
  MLIRArmDialect
  ${dialect_libs}
  ${conversion_libs}
  ${extension_libs}
  MLIRRegisterAllDialects
  MLIRRegisterAllExtensions
  MLIRRegisterAllPasses
  MLIROptLib
  MLIRIR
  MLIRSupport
)

mlir_check_all_link_libraries(arm-opt)
