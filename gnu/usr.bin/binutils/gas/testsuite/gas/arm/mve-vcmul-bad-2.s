.syntax unified
.thumb
vcmul.i16 q0, q1, q2, #0
vcmul.f64 q0, q1, q2, #0
vcmul.f32 q0, q1, q2, #20
vcmul.f32 q0, q1, q0, #0
vcmul.f32 q0, q0, q1, #0

.irp cond, eq, ne, gt, ge, lt, le

it \cond
vcmul.f32 q0, q1, q2, #0

.endr

it eq
vcmuleq.f32 q0, q1, q2, #0
vcmuleq.f32 q0, q1, q2, #0
vpst
vcmuleq.f32 q0, q1, q2, #0
vcmult.f32 q0, q1, q2, #0
vpst
vcmul.f32 q0, q1, q2, #0
