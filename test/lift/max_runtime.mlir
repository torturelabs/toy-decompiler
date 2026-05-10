// Runtime equivalence for max() with conditional execution lifted via
// CondGuard. Lower through the LLVM-IR path, link with a multi-case
// harness, and check that signed-max behavior is preserved.
//
// RUN: %arm-lift %S/max.elf | \
// RUN:   %arm-opt --arm-to-arith --convert-arith-to-llvm \
// RUN:           --convert-cf-to-llvm --convert-func-to-llvm \
// RUN:           --reconcile-unrealized-casts | \
// RUN:   %mlir-translate --mlir-to-llvmir > %t.ll
// RUN: %clang -Wno-override-module %S/max_harness.c %t.ll -o %t.exe
// RUN: %t.exe | %FileCheck %s

// CHECK: max(40, 2) = 40
// CHECK: max(2, 40) = 40
// CHECK: max(7, 7) = 7
// CHECK: max(-3, 5) = 5
// CHECK: max(-100, -50) = -50
// CHECK: max(0, 0) = 0
// CHECK: max(2147483647, 1) = 2147483647
