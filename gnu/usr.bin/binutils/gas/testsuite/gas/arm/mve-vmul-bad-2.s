.macro cond size, lastreg
.irp cond, eq, ne, gt, ge, lt, le
it \cond
vmul.\size q0, q1, \lastreg
.endr
.endm

.syntax unified
.thumb
vmul.f64 q0, q1, q2
vmul.f64 q0, q1, r2
vmul.i64 q0, q1, q2
vmul.i64 q0, q1, r2
vmul.f16 q0, q1, pc
vmul.f16 q0, q1, pc
vmul.f16 q0, q1, sp
vmul.f16 q0, q1, sp
vmul.i32 q0, q1, pc
vmul.i32 q0, q1, pc
vmul.i32 q0, q1, sp
vmul.i32 q0, q1, sp
cond i8 q2
cond i16 r2
cond f16 q2
cond f32 r2
it eq
vmuleq.f16 q0, q1, q2
vmuleq.f16 q0, q1, q2
vpst
vmuleq.f16 q0, q1, q2
vmult.f16 q0, q1, q2
vpst
vmul.f16 q0, q1, q2
it eq
vmuleq.f32 q0, q1, r2
vmuleq.f32 q0, q1, r2
vpst
vmuleq.f32 q0, q1, r2
vmult.f32 q0, q1, r2
vpst
vmul.f32 q0, q1, r2
