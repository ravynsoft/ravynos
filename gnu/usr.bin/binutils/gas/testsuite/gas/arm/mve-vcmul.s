.syntax unified
.thumb
.macro vcmul_q0 data, op4
.irp op2, q1, q2, q4, q7
.irp op3, q1, q2, q4, q7
vcmul.\data q0, \op2, \op3, \op4
.endr
.endr
.endm

.macro vcmul_q1 data, op4
.irp op2, q0, q2, q4, q7
.irp op3, q0, q2, q4, q7
vcmul.\data q1, \op2, \op3, \op4
.endr
.endr
.endm

.macro vcmul_q2 data, op4
.irp op2, q0, q1, q4, q7
.irp op3, q0, q1, q4, q7
vcmul.\data q2, \op2, \op3, \op4
.endr
.endr
.endm

.macro vcmul_q4 data, op4
.irp op2, q0, q1, q2, q7
.irp op3, q0, q1, q2, q7
vcmul.\data q4, \op2, \op3, \op4
.endr
.endr
.endm

.macro vcmul_q7 data, op4
.irp op2, q0, q1, q2, q4
.irp op3, q0, q1, q2, q4
vcmul.\data q7, \op2, \op3, \op4
.endr
.endr
.endm

.irp data, f16, f32
.irp op4, #0, #90, #180, #270
vcmul_q0 \data, \op4
vcmul_q1 \data, \op4
vcmul_q2 \data, \op4
vcmul_q4 \data, \op4
vcmul_q7 \data, \op4
.endr
.endr
vpstete
vcmult.f16 q7, q7, q7, #0
vcmule.f16 q6, q3, q1, #90
vcmult.f32 q2, q5, q4, #180
vcmule.f32 q5, q6, q7, #270
