// RUN: %arm-lift %S/add.elf | %arm-opt --arm-to-arith | %FileCheck %s

// End-to-end: ARM32 ELF → arm dialect → arith dialect.
//
// Source (test/lift/add.s):
//     add:
//         add r0, r0, r1
//         bx lr
//
// CHECK-LABEL: func.func @add
// CHECK:         %[[SUM:.*]] = arith.addi %arg0, %arg1 : i32
// CHECK:         return %[[SUM]] : i32
