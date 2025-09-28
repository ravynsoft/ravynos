.syntax unified
.thumb

.irp cond, eq, ne, gt, ge, lt, le
.irp size, .f16.s16, .s16.f16, .f16.u16, .u16.f16, .f32.s32, .s32.f32, .f32.u32, .u32.f32

it \cond
vcvt\size q0, q1

.endr
.endr

vcvt.u64.f64 q0, q1
vcvt.f64.u64 q0, q1
vcvt.s64.f64 q0, q1
vcvt.f64.s64 q0, q1
it eq
vcvteq.f32.s32 q0, q1
vcvteq.f32.s32 q0, q1
vpst
vcvteq.f32.s32 q0, q1
vpst
vcvt.f32.s32 q0, q1
vcvtt.f32.s32 q0, q1
