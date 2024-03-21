.syntax unified
.thumb

.irp conv, .f16.s16, .f16.u16, .s16.f16, .u16.f16, .f32.s32, .f32.u32, .s32.f32, .u32.f32
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
vcvt\conv \op1, \op2
.endr
.endr
.endr

vpsttee
vcvtt.f16.s16 q0, q1
vcvtt.f16.u16 q1, q2
vcvte.s16.f16 q2, q3
vcvte.u16.f16 q3, q4
vpsttee
vcvtt.f32.s32 q4, q5
vcvtt.f32.u32 q5, q6
vcvte.s32.f32 q6, q7
vcvte.u32.f32 q7, q0
@vcvt.u64.f64 q0, q1
