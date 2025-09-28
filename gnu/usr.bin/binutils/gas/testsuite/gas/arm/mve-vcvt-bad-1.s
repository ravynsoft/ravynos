.syntax unified
.thumb
vcvt.f16.s16 q0, q1, #0
vcvt.s16.f16 q0, q1, #0
vcvt.f16.u16 q0, q1, #0
vcvt.u16.f16 q0, q1, #0
vcvt.f16.s16 q0, q1, #17
vcvt.s16.f16 q0, q1, #17
vcvt.f16.u16 q0, q1, #17
vcvt.u16.f16 q0, q1, #17
vcvt.f32.s32 q0, q1, #0
vcvt.s32.f32 q0, q1, #0
vcvt.f32.u32 q0, q1, #0
vcvt.u32.f32 q0, q1, #0
vcvt.f32.s32 q0, q1, #33
vcvt.s32.f32 q0, q1, #33
vcvt.f32.u32 q0, q1, #33
vcvt.u32.f32 q0, q1, #33

.irp cond, eq, ne, gt, ge, lt, le
.irp size, .f16.s16, .s16.f16, .f16.u16, .u16.f16, .f32.s32, .s32.f32, .f32.u32, .u32.f32

it \cond
vcvt\size q0, q1, #1

.endr
.endr

vcvt.f64.u64 q0, q1, #1
vcvt.u64.f64 q0, q1, #1
vcvt.f64.s64 q0, q1, #1
vcvt.s64.f64 q0, q1, #1
it eq
vcvteq.f32.u32 q0, q1, #1
vcvteq.f32.u32 q0, q1, #1
vpst
vcvteq.f32.u32 q0, q1, #1
vpst
vcvt.f32.u32 q0, q1, #1
vcvtt.f32.u32 q0, q1, #1
