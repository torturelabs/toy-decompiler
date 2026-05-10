// Runtime equivalence for pick() — multi-block CFG via cf.br + cf.cond_br
// (no conditional execution involved this time; pure inter-block control
// flow). Lower through the LLVM-IR path and run.
//
// RUN: %arm-lift %S/pick.elf | \
// RUN:   %arm-opt --arm-to-arith --convert-arith-to-llvm \
// RUN:           --convert-cf-to-llvm --convert-func-to-llvm \
// RUN:           --reconcile-unrealized-casts | \
// RUN:   %mlir-translate --mlir-to-llvmir > %t.ll
// RUN: %clang -Wno-override-module %S/pick_harness.c %t.ll -o %t.exe
// RUN: %t.exe | %FileCheck %s

// CHECK: pick(1, 100, 200) = 100
// CHECK: pick(0, 100, 200) = 100
// CHECK: pick(-1, 100, 200) = 200
// CHECK: pick(-99, 7, 13) = 13
// CHECK: pick(-2147483648, 1, 2) = 2
// CHECK: pick(2147483647, 1, 2) = 1
