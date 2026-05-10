find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(DECODETREE   ${CMAKE_CURRENT_SOURCE_DIR}/decodetree.py)
set(A32_DECODE   ${CMAKE_CURRENT_SOURCE_DIR}/a32.decode)
set(GEN_STUBS    ${PROJECT_SOURCE_DIR}/scripts/gen_trans_stubs.py)

set(A32_DECODER_INC ${CMAKE_CURRENT_BINARY_DIR}/a32_decoder.c.inc)
set(A32_STUBS_INC   ${CMAKE_CURRENT_BINARY_DIR}/a32_stubs.c.inc)

add_custom_command(
  OUTPUT ${A32_DECODER_INC}
  COMMAND ${Python3_EXECUTABLE} ${DECODETREE}
          --static-decode disas_a32
          -o ${A32_DECODER_INC}
          ${A32_DECODE}
  DEPENDS ${DECODETREE} ${A32_DECODE}
  COMMENT "decodetree: generating A32 decoder dispatch"
  VERBATIM
)

set(A32_EXCLUDE ${PROJECT_SOURCE_DIR}/tools/arm-lift/SemanticsExcluded.txt)

add_custom_command(
  OUTPUT ${A32_STUBS_INC}
  COMMAND ${Python3_EXECUTABLE} ${GEN_STUBS}
          --exclude ${A32_EXCLUDE}
          ${A32_DECODER_INC} ${A32_STUBS_INC}
  DEPENDS ${A32_DECODER_INC} ${GEN_STUBS} ${A32_EXCLUDE}
  COMMENT "Generating A32 trans_* stubs"
  VERBATIM
)

add_custom_target(armDecoderTablesGen
  DEPENDS ${A32_DECODER_INC} ${A32_STUBS_INC}
)

add_library(armDecoderTables INTERFACE)
target_include_directories(armDecoderTables INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(armDecoderTables armDecoderTablesGen)
