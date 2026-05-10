// Headline runtime test for M9 (signature recovery): the lifted function
// is rewritten down from 20 args to 3 (sel, a, b) by dropping unused
// register-file slots. Compile + run the recovered C against a 3-arg
// harness and check signed-pick behavior.
//
// RUN: %arm-lift %S/pick.elf | \
// RUN:   %arm-opt --arm-to-arith --lift-cf-to-scf \
// RUN:           --arm-recover-function-signature \
// RUN:           --convert-scf-to-emitc --convert-arith-to-emitc \
// RUN:           --convert-func-to-emitc | \
// RUN:   %mlir-translate --mlir-to-cpp > %t.c
// RUN: gcc -include stdint.h -include stdbool.h -x c %t.c \
// RUN:     %S/pick_recovered_harness.c -o %t.exe
// RUN: %t.exe | %FileCheck %s

// CHECK: pick(1, 100, 200) = 100
// CHECK: pick(0, 100, 200) = 100
// CHECK: pick(-1, 100, 200) = 200
// CHECK: pick(-99, 7, 13) = 13
// CHECK: pick(-2147483648, 1, 2) = 2
// CHECK: pick(2147483647, 1, 2) = 1
