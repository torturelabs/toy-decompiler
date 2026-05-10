// RUN: %arm-lift %S/pick.elf | %FileCheck %s

// Source (test/lift/pick.s):
//     pick:
//         cmp r0, #0           ; entry block
//         bge .La              ; conditional inter-block branch
//         mov r0, r2           ; ^else: r0 = b
//         b .Ldone             ; unconditional inter-block branch
//     .La:
//         mov r0, r1           ; ^then: r0 = a (falls through to .Ldone)
//     .Ldone:
//         bx lr                ; ^join: return r0
//
// The lifter discovers four block leaders (entry, fall-through after BGE,
// BGE target, B target) and emits a 4-block CFG: cf.cond_br for the BGE,
// cf.br for the unconditional B and the natural fall-through, func.return
// for the BX.

// CHECK-LABEL: func.func @pick

// Entry: CMP r0, #0 → flags. GE = (N == V).
// CHECK:         arith.cmpi eq

// cf.cond_br dispatches on the GE predicate. Both successors get the same
// register-file state.
// CHECK:         cf.cond_br %{{[0-9]+}}, ^bb{{[0-9]+}}({{.*}}), ^bb{{[0-9]+}}(

// One block ends in an unconditional cf.br (the explicit `b .Ldone`).
// CHECK:         cf.br ^bb{{[0-9]+}}(

// The other block ends in an automatic fall-through cf.br (between the
// `mov r0, r1` block and `.Ldone`).
// CHECK:         cf.br ^bb{{[0-9]+}}(

// Join block returns the merged r0.
// CHECK:         return %{{[0-9]+}} : i32
