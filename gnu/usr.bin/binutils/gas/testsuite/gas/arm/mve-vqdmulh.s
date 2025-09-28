.syntax unified
.thumb
.irp data, s8, s16, s32
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, q0, q1, q2, q4, q7
vqdmulh.\data \op1, \op2, \op3
vqrdmulh.\data \op1, \op2, \op3
.endr
.irp op3, r0, r1, r2, r4, r7, r8, r10, r12, r14
vqdmulh.\data \op1, \op2, \op3
vqrdmulh.\data \op1, \op2, \op3
.endr
.endr
.endr
.endr
vpstete
vqdmulht.s8 q0, q1, q2
vqdmulhe.s8 q0, q1, q2
vqdmulht.s8 q0, q1, r2
vqdmulhe.s8 q0, q1, r2
vpstete
vqrdmulht.s32 q0, q1, q2
vqrdmulhe.s32 q0, q1, q2
vqrdmulht.s32 q0, q1, r2
vqrdmulhe.s32 q0, q1, r2
