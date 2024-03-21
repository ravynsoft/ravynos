.syntax unified
.thumb
.irp data, i8, i16, i32
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
vclz.\data \op1, \op2
.endr
.endr
.endr
vpstete
vclzt.i8 q0, q1
vclze.i16 q0, q1
vclzt.i32 q2, q1
vclze.i8 q2, q1
