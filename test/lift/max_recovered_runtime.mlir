// Same M9 pipeline applied to maxfn — signature recovers from 20 args to
// 2 (a, b). The C body still has the explicit NZCV flag derivation per
// CMP, but the wrapping signature is the natural shape.
//
// RUN: %arm-lift %S/max.elf | \
// RUN:   %arm-opt --arm-to-arith --lift-cf-to-scf \
// RUN:           --arm-recover-function-signature \
// RUN:           --convert-scf-to-emitc --convert-arith-to-emitc \
// RUN:           --convert-func-to-emitc | \
// RUN:   %mlir-translate --mlir-to-cpp > %t.c
// RUN: gcc -include stdint.h -include stdbool.h -x c %t.c \
// RUN:     %S/max_recovered_harness.c -o %t.exe
// RUN: %t.exe | %FileCheck %s

// CHECK: max(40, 2) = 40
// CHECK: max(2, 40) = 40
// CHECK: max(7, 7) = 7
// CHECK: max(-3, 5) = 5
// CHECK: max(-100, -50) = -50
// CHECK: max(0, 0) = 0
// CHECK: max(2147483647, 1) = 2147483647
