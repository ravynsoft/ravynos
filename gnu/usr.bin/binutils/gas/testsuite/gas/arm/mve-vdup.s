.syntax unified
.thumb
.irp data, 8, 16, 32
.irp op1, q0, q1, q2, q4, q7
.irp op2, r0, r1, r2, r4, r7, r8, r10, r11, r12, r14
vdup.\data \op1, \op2
.endr
.endr
.endr
vpstete
vdupt.8 q0, r1
vdupe.16 q7, r0
vdupt.32 q0, r14
vdupe.32 q3, r2
