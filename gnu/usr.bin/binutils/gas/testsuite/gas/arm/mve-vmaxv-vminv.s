.syntax unified
.thumb
.macro vmaxminv op
.irp op1, r0, r1, r2, r4, r7, r8, r10, r14
.irp op2, q0, q1, q2, q4, q7
\op \op1, \op2
.endr
.endr
.endm

.irp data, u8, s8, u16, s16, u32, s32
vmaxminv vmaxv.\data
vmaxminv vminv.\data
.endr

.irp data, s8, s16, s32
vmaxminv vmaxav.\data
vmaxminv vminav.\data
.endr

vpstete
vmaxvt.u8 r0, q7
vmaxve.s16 r8, q2
vmaxavt.s8 r7, q3
vmaxave.s16 r8, q2
vpstete
vminvt.u16 r0, q7
vminve.s32 r8, q2
vminavt.s16 r7, q3
vminave.s32 r8, q2
