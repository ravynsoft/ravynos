.syntax unified
.thumb

.irp lastreg, q1, r1
.irp cond, eq, ne, gt, ge, lt, le
.irp size, .
it \cond
vpt.i8 eq, q0, \lastreg
vaddt.i32 q0, q1, q2
.endr
.endr
.endr


vpt.i8 eq, q0, sp
vaddt.i32 q0, q1, q2
vpt.i64 eq, q0, q1
vpt.f16 eq, q0, q1
vpt.f32 eq, q0, q1
vpt.f64 eq, q0, q1
it eq
vpteq.i8 eq, q0, q1
vpteq.i8 eq, q0, q1
vaddt.i32 q0, q0, q1
vpst
vptt.i8 eq, q0, q1
vptt.i8 eq, q0, q1
vaddt.i32 q0, q0, q1
vaddt.i32 q0, q0, q1
vpt.i8 eq, q0, q1
