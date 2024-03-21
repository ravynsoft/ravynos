.syntax unified
.thumb

vmaxnma.f64 q0, q1
vmaxnma.i16 q0, q1
vminnma.f64 q0, q1
vminnma.i16 q0, q1

.irp op, vmaxnma, vminnma
.irp cond, eq, ne, gt, ge, lt, le
it \cond
\op\().f16 q0, q1
.endr
.endr

it eq
vmaxnmaeq.f32 q0, q1
vmaxnmaeq.f32 q0, q1
vpst
vmaxnmaeq.f32 q0, q1
vmaxnmat.f32 q0, q1
vpst
vmaxnma.f32 q0, q1
it eq
vminnmaeq.f32 q0, q1
vminnmaeq.f32 q0, q1
vpst
vminnmaeq.f32 q0, q1
vminnmat.f32 q0, q1
vpst
vminnma.f32 q0, q1
