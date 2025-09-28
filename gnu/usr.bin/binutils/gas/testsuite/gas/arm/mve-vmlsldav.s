.syntax unified
.thumb
.irp data, s16, s32
.irp op1, r0, r2, r4, r8, r10, r12, r14
.irp op2, r1, r3, r5, r7, r9, r11
.irp op3, q0, q1, q2, q4, q7
.irp op4, q0, q1, q2, q4, q7
vmlsldav.\data \op1, \op2, \op3, \op4
vmlsldava.\data \op1, \op2, \op3, \op4
vmlsldavx.\data \op1, \op2, \op3, \op4
vmlsldavax.\data \op1, \op2, \op3, \op4
.endr
.endr
.endr
.endr
.endr

vpstete
vmlsldavt.s16 r0, r1, q2, q3
vmlsldavae.s16 r12, r11, q4, q3
vmlsldavxt.s32 r4, r5, q1, q6
vmlsldavaxe.s32 r14, r11, q7, q7

