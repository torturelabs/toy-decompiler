// Headline runtime test for M5: the `pick(sel, a, b)` ARM32 function lifts
// → `arm` → `arith+cf` → `arith+scf` (structure recovered) → `emitc` → C
// source → host gcc → native binary, and the binary still computes the
// signed pick correctly.
//
// RUN: %arm-lift %S/pick.elf | \
// RUN:   %arm-opt --arm-to-arith --lift-cf-to-scf --convert-scf-to-emitc \
// RUN:           --convert-arith-to-emitc --convert-func-to-emitc | \
// RUN:   %mlir-translate --mlir-to-cpp > %t.c
// RUN: gcc -include stdint.h -include stdbool.h -x c %t.c \
// RUN:     %S/pick_harness.c -o %t.exe
// RUN: %t.exe | %FileCheck %s

// CHECK: pick(1, 100, 200) = 100
// CHECK: pick(0, 100, 200) = 100
// CHECK: pick(-1, 100, 200) = 200
// CHECK: pick(-99, 7, 13) = 13
// CHECK: pick(-2147483648, 1, 2) = 2
// CHECK: pick(2147483647, 1, 2) = 1
