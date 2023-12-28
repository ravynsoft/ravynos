.syntax unified
.thumb
.irp data, s8, u8, s16, u16, s32, u32
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, q0, q1, q2, q4, q7
vqadd.\data \op1, \op2, \op3
vqsub.\data \op1, \op2, \op3
.endr
.irp op3, r0, r1, r2, r4, r7, r8, r10, r12, r14
vqadd.\data \op1, \op2, \op3
vqsub.\data \op1, \op2, \op3
.endr
.endr
.endr
.endr
vpstete
vqaddt.s8 q0, q1, q2
vqadde.s8 q0, q1, q2
vqsubt.u8 q0, q1, q2
vqsube.u8 q0, q1, q2
vpstete
vqaddt.s16 q0, q1, r3
vqadde.s16 q0, q1, r3
vqsubt.u32 q0, q1, r11
vqsube.u32 q0, q1, r11
