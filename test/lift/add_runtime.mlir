// Runtime-equivalence oracle: lift the ARM32 ELF, lower to native, link
// against a tiny C harness that calls `lifted(40, 2, ...)` and exits 0
// iff the result is 42.
//
// RUN: %arm-lift %S/add.elf | \
// RUN:   %arm-opt --arm-to-arith --convert-arith-to-llvm \
// RUN:           --convert-func-to-llvm --reconcile-unrealized-casts | \
// RUN:   %mlir-translate --mlir-to-llvmir > %t.ll
// RUN: %clang -Wno-override-module %S/add_harness.c %t.ll -o %t.exe
// RUN: %t.exe | %FileCheck %s

// CHECK: lifted(40, 2) = 42
