// RUN: %arm-lift %S/pick.elf | \
// RUN:   %arm-opt --arm-to-arith --lift-cf-to-scf | %FileCheck %s

// MLIR's `--lift-cf-to-scf` (M5) collapses the 4-block CFG produced by
// the lifter into a single `scf.if`. Most of the dead register-state
// plumbing falls out via dataflow analysis; only the LT predicate and the
// pick-result remain.
//
// CHECK-LABEL: func.func @pick

// The flag-derivation chain reduces to a single signed compare against 0.
// CHECK:         arith.cmpi slt

// The recovered structure is one if-with-yields, no remaining cf ops.
// CHECK-NOT:     cf.br
// CHECK-NOT:     cf.cond_br
// CHECK:         scf.if
// CHECK:           scf.yield
// CHECK:         } else {
// CHECK:           scf.yield
// CHECK:         }
// CHECK:         return
