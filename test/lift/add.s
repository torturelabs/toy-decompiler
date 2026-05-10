.syntax unified
.arch armv7-a
.text
.global add
.type add, %function
add:
    add r0, r0, r1
    bx lr
.size add, .-add
