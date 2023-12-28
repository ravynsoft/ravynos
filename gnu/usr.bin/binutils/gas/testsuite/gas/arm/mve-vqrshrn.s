.syntax unified
.thumb

.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, #1, #2, #4, #7, #8
.irp data, s16, u16
vqrshrnt.\data \op1, \op2, \op3
vqrshrnb.\data \op1, \op2, \op3
.endr
vqrshrunt.s16 \op1, \op2, \op3
vqrshrunb.s16 \op1, \op2, \op3
.endr
.endr
.endr
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, #1, #2, #4, #7, #8, #10, #13, #15, #16
.irp data, s32, u32
vqrshrnt.\data \op1, \op2, \op3
vqrshrnb.\data \op1, \op2, \op3
.endr
vqrshrunt.s32 \op1, \op2, \op3
vqrshrunb.s32 \op1, \op2, \op3
.endr
.endr
.endr

vpstete
vqrshrntt.u16 q0, q1, #1
vqrshrnte.u32 q7, q7, #16
vqrshrnbt.s16 q7, q7, #8
vqrshrnbe.s32 q0, q1, #1
vpstete
vqrshruntt.s16 q0, q1, #1
vqrshrunte.s32 q7, q7, #16
vqrshrunbt.s16 q7, q7, #8
vqrshrunbe.s32 q0, q1, #1
