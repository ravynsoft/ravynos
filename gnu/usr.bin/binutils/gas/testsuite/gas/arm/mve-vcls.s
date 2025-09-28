.syntax unified
.thumb
.irp data, s8, s16, s32
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
vcls.\data \op1, \op2
.endr
.endr
.endr

vpstete
vclst.s8 q0, q1
vclse.s16 q0, q1
vclst.s32 q2, q1
vclse.s8 q2, q1
