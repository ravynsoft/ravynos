.syntax unified
.thumb
.irp data, s8, s16, s32
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
vmaxa.\data \op1, \op2
vmina.\data \op1, \op2
.endr
.endr
.endr

vpstete
vmaxat.s8 q0, q1
vmaxae.s16 q7, q7
vminat.s32 q0, q1
vminae.s16 q7, q7
