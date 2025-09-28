.syntax unified
.thumb
vcadd.f16 q0, q1, q2, #90
vcadd.64 q0, q1, q2, #90
vcadd.i32 q0, q1, q2, #180
vcadd.i32 q0, q1, q2, #0
vcadd.i32 q0, q1, q0, #90

.irp cond, eq, ne, gt, ge, lt, le

it \cond
vcadd.i16 q0, q1, q2, #90

.endr

it eq
vcaddeq.i16 q0, q1, q2, #90
vcaddeq.i16 q0, q1, q2, #90
vpst
vcaddeq.i16 q0, q1, q2, #90
vcaddt.i16 q0, q1, q2, #90
vpst
vcadd.i16 q0, q1, q2, #90
