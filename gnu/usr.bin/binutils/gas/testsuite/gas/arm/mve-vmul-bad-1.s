.syntax unified
.thumb
vmul.f16 q0, q1, q2
vmul.f16 q0, q1, r2
vmul.f32 q0, q1, q2
vmul.f32 q0, q1, r2
vmul.i64 q0, q1, q2
vmul.i64 q0, q1, r2
vmul.i8 q0, q1, pc
vmul.i8 q0, q1, sp

.irp lastreg, q2, r2
.irp cond, eq, ne, gt, ge, lt, le
it \cond
vmul.i16 q0, q1, \lastreg
.endr
.endr


it eq
vmuleq.i32 q0, q1, q2
vmuleq.i32 q0, q1, q2
vpst
vmuleq.i32 q0, q1, q2
vmult.i32 q0, q1, q2
vpst
vmul.i32 q0, q1, q2
it eq
vmuleq.i32 q0, q1, r2
vmuleq.i32 q0, q1, r2
vpst
vmuleq.i32 q0, q1, r2
vmult.i32 q0, q1, r2
vpst
vmul.i32 q0, q1, r2
