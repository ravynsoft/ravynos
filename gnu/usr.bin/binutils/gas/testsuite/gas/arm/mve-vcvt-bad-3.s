.syntax unified
.thumb

.irp top, t, b
.irp cond, eq, ne, gt, ge, lt, le
.irp size, .f16.f32, .f32.f16

it \cond
vcvt\top\size q0, q1

.endr
.endr
.endr

vcvt.f64.f16 q0, q1
vcvt.f64.f32 q0, q1
vcvt.f16.f64 q0, q1
vcvt.f32.f64 q0, q1
it eq
vcvtteq.f16.f32 q0, q1
vcvtteq.f16.f32 q0, q1
vpst
vcvtteq.f16.f32 q0, q1
vcvttt.f16.f32 q0, q1
vpst
vcvtt.f16.f32 q0, q1
it eq
vcvtbeq.f16.f32 q0, q1
vcvtbeq.f16.f32 q0, q1
vpst
vcvtbeq.f16.f32 q0, q1
vcvtbt.f16.f32 q0, q1
vpst
vcvtb.f16.f32 q0, q1
