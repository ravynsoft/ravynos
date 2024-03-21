.syntax unified
.thumb
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, q0, q1, q2, q4, q7
vpsel \op1, \op2, \op3
.irp data, s8, i8, u16, f16, i32, f32
vpsel.\data \op1, \op2, \op3
.endr
.endr
.endr
.endr
vpste
vpselt.i8 q0, q1, q2
vpsele q0, q0, q0
