.syntax unified
.thumb
.irp op1, q0, q1, q2, q4, q7
.irp op2, r0, r1, r2, r4, r7, r8, r10, r12, r14
.irp op3, #1, #2, #4, #7, #8, #16, #24, #31, #32
vshlc \op1, \op2, \op3
.endr
.endr
.endr
vpste
vshlct q0, r1, #1
vshlce q5, r12, #17
