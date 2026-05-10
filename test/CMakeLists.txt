find_program(FILECHECK_PATH NAMES FileCheck REQUIRED)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.py.in
  ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg.py
  @ONLY
)

set(TOY_DECOMPILER_TEST_DEPENDS
  arm-opt
  arm-lift
)

find_program(LIT_EXECUTABLE NAMES lit llvm-lit REQUIRED)

add_custom_target(check-toy-decompiler
  COMMAND ${LIT_EXECUTABLE} -v ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${TOY_DECOMPILER_TEST_DEPENDS}
  USES_TERMINAL
)
