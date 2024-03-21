.syntax unified
.thumb
.irp data, i8, i16, f16
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, q0, q1, q2, q4, q7
.irp op4, #90, #270
vcadd.\data \op1, \op2, \op3, \op4
.endr
.endr
.endr
.endr
.endr

.macro vcadd_q0 data, op2, op4
.irp op3, q1, q2, q4, q7
vcadd.\data q0, \op2, \op3, \op4
.endr
.endm

.macro vcadd_q1 data, op2, op4
.irp op3, q0, q2, q4, q7
vcadd.\data q1, \op2, \op3, \op4
.endr
.endm

.macro vcadd_q2 data, op2, op4
.irp op3, q0, q1, q4, q7
vcadd.\data q2, \op2, \op3, \op4
.endr
.endm

.macro vcadd_q3 data, op2, op4
.irp op3, q0, q1, q2, q4, q7
vcadd.\data q3, \op2, \op3, \op4
.endr
.endm

.macro vcadd_q4 data, op2, op4
.irp op3, q0, q1, q2, q3, q7
vcadd.\data q4, \op2, \op3, \op4
.endr
.endm

.macro vcadd_q6 data, op2, op4
.irp op3, q0, q1, q2, q4, q7
vcadd.\data q6, \op2, \op3, \op4
.endr
.endm

.macro vcadd_q7 data, op2, op4
.irp op3, q0, q1, q2, q4, q5
vcadd.\data q7, \op2, \op3, \op4
.endr
.endm

.irp data, i32, f32
.irp op2, q0, q1, q2, q4, q7
.irp op4, #90, #270
vcadd_q0 \data, \op2, \op4
vcadd_q1 \data, \op2, \op4
vcadd_q2 \data, \op2, \op4
vcadd_q3 \data, \op2, \op4
vcadd_q4 \data, \op2, \op4
vcadd_q6 \data, \op2, \op4
vcadd_q7 \data, \op2, \op4
.endr
.endr
.endr

vpstete
vcaddt.i8 q0, q1, q2, #90
vcadde.i8 q7, q7, q7, #270
vcaddt.i16 q0, q1, q2, #90
vcadde.i16 q0, q1, q2, #270
vpstete
vcaddt.i32 q0, q1, q2, #90
vcadde.i32 q0, q1, q2, #270
vcaddt.f16 q0, q1, q2, #90
vcadde.f32 q0, q1, q2, #270
