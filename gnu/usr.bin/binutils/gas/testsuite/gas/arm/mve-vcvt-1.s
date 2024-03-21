.syntax unified
.thumb

.macro all_vcvt conv, imm
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
vcvt\conv \op1, \op2, #\imm
.endr
.endr
.endm

.irp conv, .f16.s16, .f16.u16, .s16.f16, .u16.f16
.irp imm, 1, 8, 12, 14, 15, 16
all_vcvt \conv, \imm
.endr
.endr

.irp conv, .f32.s32, .f32.u32, .s32.f32, .u32.f32
.irp imm, 1, 16, 24, 28, 30, 31, 32
all_vcvt \conv, \imm
.endr
.endr

vpsttee
vcvtt.f16.s16 q0, q1, #1
vcvtt.f16.u16 q1, q2, #2
vcvte.s16.f16 q2, q3, #3
vcvte.u16.f16 q3, q4, #4
vpsttee
vcvtt.f32.s32 q4, q5, #5
vcvtt.f32.u32 q5, q6, #21
vcvte.s32.f32 q6, q7, #22
vcvte.u32.f32 q7, q0, #23
