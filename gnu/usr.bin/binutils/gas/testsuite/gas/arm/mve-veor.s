.syntax unified
.thumb
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, q0, q1, q2, q4, q7
.irp data, u8, s8, i8, u16, f16, s32, f32
veor.\data \op1, \op2, \op3
.endr
veor \op1, \op2, \op3
.endr
.endr
.endr
vpstete
veort.s8 q0, q1, q2
veore q0, q1, q2
veort.f16 q0, q1, q2
veore.i32 q0, q1, q2
