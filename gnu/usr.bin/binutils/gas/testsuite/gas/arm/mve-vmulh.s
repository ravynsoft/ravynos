.syntax unified
.thumb
.irp data, s8, u8, s16, u16, s32, u32
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, q0, q1, q2, q4, q7
vmulh.\data \op1, \op2, \op3
vrmulh.\data \op1, \op2, \op3
.endr
.endr
.endr
.endr
vpstete
vmulht.u8 q0, q1, q2
vmulhe.s16 q2, q2, q6
vrmulht.u32 q3, q0, q7
vrmulhe.s8 q0, q1, q2
