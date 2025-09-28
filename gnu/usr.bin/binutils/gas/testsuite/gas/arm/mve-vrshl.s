.syntax unified
.thumb
.irp data, u8, s8, u16, s16, u32, s32
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, q0, q1, q2, q4, q7
vrshl.\data \op1, \op2, \op3
.endr
.endr
.irp op2, r0, r1, r2, r4, r7, r8, r10, r12, r14
vrshl.\data \op1, \op2
.endr
.endr
.endr
vpstete
vrshlt.s8 q0, q1, q2
vrshle.u16 q7, q7, q7
vrshlt.s32 q0, r2
vrshle.u8 q7, lr
