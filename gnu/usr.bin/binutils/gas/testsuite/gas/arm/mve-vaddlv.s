.syntax unified
.thumb

.irp data, .s32, .u32
.irp op1, r0, r2, r4, r8, r10, r12, r14
.irp op2, r1, r3, r5, r7, r9, r11
.irp op3, q0, q1, q2, q4, q7
vaddlv\data \op1, \op2, \op3
vaddlva\data \op1, \op2, \op3
.endr
.endr
.endr
.endr

vpsttee
vaddlvt.s32 r0, r1, q0
vaddlvat.s32 r0, r1, q0
vaddlve.u32 r0, r1, q0
vaddlvae.u32 r0, r1, q0
