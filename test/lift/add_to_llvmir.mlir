// RUN: %arm-lift %S/add.elf | \
// RUN:   %arm-opt --arm-to-arith --convert-arith-to-llvm \
// RUN:           --convert-func-to-llvm --reconcile-unrealized-casts | \
// RUN:   %mlir-translate --mlir-to-llvmir | %FileCheck %s

// Full lower-to-LLVM-IR path. The lifted ARM32 function should still be a
// recognizable add of the first two arguments (which correspond to r0 and r1
// in the ARM AAPCS calling convention).
//
// CHECK-LABEL: define i32 @add
// CHECK:         %[[SUM:.*]] = add i32 %0, %1
// CHECK:         ret i32 %[[SUM]]
