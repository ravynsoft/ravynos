.syntax unified
.thumb

vmaxnm.f64 q0, q1, q2
vmaxnm.i16 q0, q1, q2
vminnm.f64 q0, q1, q2
vminnm.i16 q0, q1, q2

.irp op, vmaxnm, vminnm
.irp cond, eq, ne, gt, ge, lt, le
it \cond
\op\().f16 q0, q1, q2
.endr
.endr

it eq
vmaxnmeq.f32 q0, q1, q2
vmaxnmeq.f32 q0, q1, q2
vpst
vmaxnmeq.f32 q0, q1, q2
vmaxnmt.f32 q0, q1, q2
vpst
vmaxnm.f32 q0, q1, q2
it eq
vminnmeq.f32 q0, q1, q2
vminnmeq.f32 q0, q1, q2
vpst
vminnmeq.f32 q0, q1, q2
vminnmt.f32 q0, q1, q2
vpst
vminnm.f32 q0, q1, q2
