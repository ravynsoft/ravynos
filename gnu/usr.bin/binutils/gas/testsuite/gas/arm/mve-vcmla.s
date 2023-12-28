.syntax unified
.thumb

.macro vcmla_q0 data, op4
.irp op2, q1, q2, q4, q7
.irp op3, q1, q2, q4, q7
vcmla.\data q0, \op2, \op3, \op4
.endr
.endr
.endm

.macro vcmla_q1 data, op4
.irp op2, q0, q2, q4, q7
.irp op3, q0, q2, q4, q7
vcmla.\data q1, \op2, \op3, \op4
.endr
.endr
.endm

.macro vcmla_q2 data, op4
.irp op2, q0, q1, q4, q7
.irp op3, q0, q1, q4, q7
vcmla.\data q2, \op2, \op3, \op4
.endr
.endr
.endm

.macro vcmla_q4 data, op4
.irp op2, q0, q1, q2, q7
.irp op3, q0, q1, q2, q7
vcmla.\data q4, \op2, \op3, \op4
.endr
.endr
.endm

.macro vcmla_q7 data, op4
.irp op2, q0, q1, q2, q4
.irp op3, q0, q1, q2, q4
vcmla.\data q7, \op2, \op3, \op4
.endr
.endr
.endm

.irp data, f16, f32
.irp op4, #0, #90, #180, #270
vcmla_q0 \data, \op4
vcmla_q1 \data, \op4
vcmla_q2 \data, \op4
vcmla_q4 \data, \op4
vcmla_q7 \data, \op4
.endr
.endr
vpstete
vcmlat.f16 q0, q1, q2, #0
vcmlae.f16 q7, q7, q7, #0
vcmlat.f32 q0, q1, q2, #0
vcmlae.f32 q0, q1, q2, #90
