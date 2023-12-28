.syntax unified
.thumb
.irp data, u8, s8, u16, s16, u32, s32
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, r0, r1, r2, r4, r7, r8, r10, r12, r14
vmlas.\data \op1, \op2, \op3
.endr
.endr
.endr
.endr
vpste
vmlast.s8 q0, q1, r2
vmlase.u16 q2, q5, r6
