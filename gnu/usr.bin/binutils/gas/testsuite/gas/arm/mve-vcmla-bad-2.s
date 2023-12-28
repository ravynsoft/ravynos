.syntax unified
.thumb
vcmla.f16 q0, q1, q2, #20
vcmla.f32 q0, q0, q1, #0
vcmla.f32 q0, q1, q0, #0
vcmla.f64 q0, q1, q2, #0
vcmla.i16 q0, q1, q2, #0

.irp cond, eq, ne, gt, ge, lt, le

it \cond
vcmla.f32 q0, q1, q2, #0

.endr

it eq
vcmlaeq.f16 q0, q1, q2, #0
vcmlaeq.f16 q0, q1, q2, #0
vpst
vcmlaeq.f16 q0, q1, q2, #0
vcmlat.f16 q0, q1, q2, #0
vpst
vcmla.f16 q0, q1, q2, #0
