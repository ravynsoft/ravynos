.syntax unified
.thumb
vcadd.f64 q0, q1, q2, #90
vcadd.f32 q0, q1, q2, #180
vcadd.f32 q0, q1, q2, #0
vcadd.f32 q0, q1, q0, #90

.irp cond, eq, ne, gt, ge, lt, le

it \cond
vcadd.f32 q0, q1, q2, #90

.endr

it eq
vcaddeq.f16 q0, q1, q2, #90
vcaddeq.f16 q0, q1, q2, #90
vpst
vcaddeq.f16 q0, q1, q2, #90
vcaddt.f16 q0, q1, q2, #90
vpst
vcadd.f16 q0, q1, q2, #90
