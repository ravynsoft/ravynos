.syntax unified
.thumb
vclz.f32 q0, q1
vclz.i64 q0, q1

.irp cond, eq, ne, gt, ge, lt, le

it \cond
vclz.i32 q0, q1

.endr

it eq
vclzeq.i16 q0, q1
vclzeq.i16 q0, q1
vpst
vclzeq.i16 q0, q1
vclzt.i16 q0, q1
vpst
vclz.i16 q0, q1
