// Runtime equivalence on the C-emission path: lift, lower to C, recompile
// the C through the host `gcc`, run, and check stdout.
//
// RUN: %arm-lift %S/add.elf | \
// RUN:   %arm-opt --arm-to-arith --convert-arith-to-emitc \
// RUN:           --convert-func-to-emitc | \
// RUN:   %mlir-translate --mlir-to-cpp > %t.c
// RUN: gcc -include stdint.h -include stdbool.h -x c %t.c \
// RUN:     %S/add_harness.c -o %t.exe
// RUN: %t.exe | %FileCheck %s

// CHECK: lifted(40, 2) = 42
