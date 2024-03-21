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

.irp data, u8, u16, u32
vmladav_all vmladav.\data
vmladav_all vmladava.\data
.endr
.irp data, s8, s16, s32
vmladav_all vmladav.\data
vmladav_all vmladava.\data
vmladav_all vmladavx.\data
vmladav_all vmladavax.\data
.endr

vpstete
vmladavt.s8 r0, q0, q1
vmladave.u8 r2, q6, q0
vmladavat.s16 r4, q5, q1
vmladavae.s32 r10, q2, q4
vpstete
vmladavxt.s8 r0, q0, q1
vmladavxe.s8 r2, q6, q0
vmladavaxt.s16 r4, q5, q1
vmladavaxe.s32 r10, q2, q4
