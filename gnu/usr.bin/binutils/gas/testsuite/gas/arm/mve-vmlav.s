.syntax unified
.thumb
.macro vmladav_all op
.irp op1, r0, r2, r4, r8, r10, r12, r14
.irp op2, q0, q1, q2, q4, q7
.irp op3, q0, q1, q2, q4, q7
\op \op1, \op2, \op3
.endr
.endr
.endr
.endm

.irp data, u8, u16, u32, s8, s16, s32
vmladav_all vmlav.\data
vmladav_all vmlava.\data
.endr

vpstete
vmlavt.s8 r0, q0, q1
vmlave.u8 r2, q6, q0
vmlavat.s16 r4, q5, q1
vmlavae.s32 r10, q2, q4
