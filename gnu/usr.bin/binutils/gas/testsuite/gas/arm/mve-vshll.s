.syntax unified
.thumb
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, #1, #2, #4, #7, #8
vshllt.s8 \op1, \op2, \op3
vshllt.u8 \op1, \op2, \op3
vshllb.s8 \op1, \op2, \op3
vshllb.u8 \op1, \op2, \op3
.endr
.irp op3, #1, #2, #4, #7, #8, #9, #10, #14, #15, #16
vshllt.s16 \op1, \op2, \op3
vshllt.u16 \op1, \op2, \op3
vshllb.s16 \op1, \op2, \op3
vshllb.u16 \op1, \op2, \op3
.endr
.endr
.endr

vpstete
vshlltt.s8 q0, q1, #1
vshllte.u16 q7, q7, #16
vshllbt.u8 q5, q0, #8
vshllbe.s16 q2, q3, #11
