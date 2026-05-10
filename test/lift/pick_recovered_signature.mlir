// FileCheck the recovered signature itself: the 20-arg `pick` from the
// lifter collapses to just 3 args after structure-recovery + dead-arg
// elimination. The function name comes from the ELF symbol table.
//
// RUN: %arm-lift %S/pick.elf | \
// RUN:   %arm-opt --arm-to-arith --lift-cf-to-scf \
// RUN:           --arm-recover-function-signature | %FileCheck %s

// CHECK-LABEL: func.func @pick(
// CHECK-SAME:    %arg0: i32, %arg1: i32, %arg2: i32) -> i32

// Body: predicate, scf.if with two yields, return the merged value.
// CHECK:         arith.cmpi slt
// CHECK:         scf.if
// CHECK:           scf.yield %arg{{[0-9]+}}
// CHECK:         } else {
// CHECK:           scf.yield %arg{{[0-9]+}}
// CHECK:         }
// CHECK:         return
