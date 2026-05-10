// RUN: %arm-lift %S/max.elf | %FileCheck %s

// Source (test/lift/max.s):
//     maxfn:
//         cmp r0, r1
//         movlt r0, r1   ; conditional execution!
//         bx lr
//
// Two interesting things here:
//   1) `cmp` derives NZCV from (r0 - r1).
//   2) `movlt` is a conditional MOV (cond = LT, the predicate fires when
//      N != V). The lifter wraps it in a `CondGuard`, splitting the entry
//      block into a body block (where the assignment runs) and an after
//      block (where the merged register state lives in block args).
// `bx lr` returns r0 — which is the post-merge value.

// CHECK-LABEL: func.func @maxfn(

// CMP r0, r1 — subtraction + flag derivation.
// CHECK:         arith.subi %arg0, %arg1
// CHECK-DAG:     arith.cmpi eq
// CHECK-DAG:     arith.cmpi slt
// CHECK-DAG:     arith.cmpi uge

// LT predicate: (N != V). The NE compare on i1 is the LT-cond i1 we feed
// into cf.cond_br.
// CHECK:         arith.cmpi ne

// CondGuard split: cond_br to ^body or ^after-with-old-state.
// CHECK:         cf.cond_br %{{[0-9]+}}, ^bb1, ^bb2(%arg0,

// Body: r0 := r1, then unconditional join carrying (%arg1, %arg1, ...).
// CHECK:       ^bb1:
// CHECK:         cf.br ^bb2(%arg1, %arg1,

// After: 20-arg merge block; first arg is the new r0 → return it.
// CHECK:       ^bb2(%{{[0-9]+}}: i32,
// CHECK:         return %{{[0-9]+}} : i32
