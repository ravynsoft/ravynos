.syntax unified
.thumb

.irp conv, .f16.f32, .f32.f16
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
vcvtt\conv \op1, \op2
vcvtb\conv \op1, \op2
.endr
.endr
.endr

vpsttee
vcvttt.f16.f32 q0, q1
vcvtbt.f16.f32 q0, q1
vcvtte.f32.f16 q5, q2
vcvtbe.f32.f16 q5, q2
