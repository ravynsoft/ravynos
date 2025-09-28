.syntax unified
.thumb
.irp data, s8, s16, s32
.irp op1, r0, r2, r4, r8, r10, r12, r14
.irp op2, q0, q1, q2, q4, q7
.irp op3, q0, q1, q2, q4, q7
vmlsdav.\data \op1, \op2, \op3
vmlsdava.\data \op1, \op2, \op3
vmlsdavx.\data \op1, \op2, \op3
vmlsdavax.\data \op1, \op2, \op3
.endr
.endr
.endr
.endr

vpstete
vmlsdavt.s8 r0, q1, q2
vmlsdave.s16 r10, q4, q6
vmlsdavxt.s32 r12, q5, q4
vmlsdavxe.s8 r2, q0, q5
vpstete
vmlsdavat.s8 r0, q1, q2
vmlsdavae.s16 r10, q4, q6
vmlsdavaxt.s32 r12, q5, q4
vmlsdavaxe.s8 r2, q0, q5
