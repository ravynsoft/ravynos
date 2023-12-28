.syntax unified
.thumb
.irp data, 8, 16, 32
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, r0, r1, r2, r4, r7, r8, r12, r14
vbrsr.\data \op1, \op2, \op3
.endr
.endr
.endr
.endr
vpstete
vbrsrt.32 q1, q0, r3
vbrsre.8 q7, q7, r5
vbrsrt.16 q4, q5, r10
vbrsre.32 q1, q3, r11
