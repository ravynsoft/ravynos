.syntax unified
.thumb
.irp data, .u8, .s8, .u16, .s16, .u32, .s32
.irp op1, r0, r2, r4, r8, r10, r12, r14
.irp op2, q0, q1, q2, q4, q7
vaddv\data \op1, \op2
vaddva\data \op1, \op2
.endr
.endr
.endr
vpstete
vaddvt.s8 r0, q0
vaddve.s16 r0, q0
vaddvat.s32 r0, q0
vaddvae.u16 r0, q0
