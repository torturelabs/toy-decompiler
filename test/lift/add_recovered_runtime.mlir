// M9 signature recovery: add() reduces from 20 args to 2 — the natural
// C signature for the lifted function.
//
// RUN: %arm-lift %S/add.elf | \
// RUN:   %arm-opt --arm-to-arith --lift-cf-to-scf \
// RUN:           --arm-recover-function-signature \
// RUN:           --convert-scf-to-emitc --convert-arith-to-emitc \
// RUN:           --convert-func-to-emitc | \
// RUN:   %mlir-translate --mlir-to-cpp > %t.c
// RUN: gcc -include stdint.h -include stdbool.h -x c %t.c \
// RUN:     %S/add_recovered_harness.c -o %t.exe
// RUN: %t.exe | %FileCheck %s

// CHECK: add(40, 2) = 42
