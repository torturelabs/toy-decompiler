// Same M5 pipeline applied to `max(a, b)` — uses conditional execution
// (CondGuard'ed MOVLT) rather than inter-block branches, but the recovered
// `scf.if` shape is the same. The C body is uglier because the toy
// derives NZCV from explicit XOR-AND-MSB patterns rather than reusing a
// single signed compare; functional equivalence is what matters.
//
// RUN: %arm-lift %S/max.elf | \
// RUN:   %arm-opt --arm-to-arith --lift-cf-to-scf --convert-scf-to-emitc \
// RUN:           --convert-arith-to-emitc --convert-func-to-emitc | \
// RUN:   %mlir-translate --mlir-to-cpp > %t.c
// RUN: gcc -include stdint.h -include stdbool.h -x c %t.c \
// RUN:     %S/max_harness.c -o %t.exe
// RUN: %t.exe | %FileCheck %s

// CHECK: max(40, 2) = 40
// CHECK: max(2, 40) = 40
// CHECK: max(7, 7) = 7
// CHECK: max(-3, 5) = 5
// CHECK: max(-100, -50) = -50
// CHECK: max(0, 0) = 0
// CHECK: max(2147483647, 1) = 2147483647
