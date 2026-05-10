add_llvm_executable(arm-lift
  main.cpp
  BlockBuilder.cpp
  Decoder.cpp
  ElfReader.cpp
  Lifter.cpp
)

add_dependencies(arm-lift armDecoderTablesGen)

llvm_update_compile_flags(arm-lift)

target_link_libraries(arm-lift
  PRIVATE
  armDecoderTables
  MLIRArmDialect
  MLIRArithDialect
  MLIRControlFlowDialect
  MLIRFuncDialect
  MLIRIR
  MLIRParser
  MLIRSupport
)

# LLVM ELF reader (`llvm::object::ELFObjectFile`) lives in the LLVMObject
# component. On Arch the static archive isn't installed, so we resolve the
# symbols through the all-in-one libLLVM shared lib that the MLIR libs
# above already pull in. If that ever changes, replace with explicit
# component linkage via `llvm_map_components_to_libnames(... object)`.

mlir_check_all_link_libraries(arm-lift)
