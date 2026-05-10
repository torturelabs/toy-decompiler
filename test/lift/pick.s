.syntax unified
.arch armv7-a
.text
.global pick
.type pick, %function
@ pick(sel, a, b): if sel >= 0 return a; else return b.
pick:
    cmp r0, #0          @ entry block
    bge .La             @ branch if sel >= 0
    mov r0, r2          @ ^bb_else: r0 = b
    b .Ldone            @ unconditional jump
.La:
    mov r0, r1          @ ^bb_then: r0 = a, fallthrough
.Ldone:
    bx lr               @ join: return r0
.size pick, .-pick
