.syntax unified
.arch armv7-a
.text
.global maxfn
.type maxfn, %function
maxfn:
    cmp r0, r1
    movlt r0, r1
    bx lr
.size maxfn, .-maxfn
