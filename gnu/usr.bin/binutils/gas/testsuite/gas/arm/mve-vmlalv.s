.syntax unified
.thumb

.irp op1, r0, r2, r4, r8, r10, r12, r14
.irp op2, r1, r3, r5, r7, r9, r11
.irp op3, q0, q1, q2, q4, q7
.irp op4, q0, q1, q2, q4, q7
.irp data, .s16, .s32, .u16, .u32
vmlalv\data \op1, \op2, \op3, \op4
vmlalva\data \op1, \op2, \op3, \op4
vmlalv\data \op1, \op2, \op3, \op4
vmlalva\data \op1, \op2, \op3, \op4
.endr
.endr
.endr
.endr
.endr

vpstete
vmlalvt.u16 r0, r1, q2, q3
vmlalve.s32 r0, r1, q2, q3
vmlalvat.u32 r0, r1, q2, q3
vmlalvae.s16 r0, r1, q2, q3
