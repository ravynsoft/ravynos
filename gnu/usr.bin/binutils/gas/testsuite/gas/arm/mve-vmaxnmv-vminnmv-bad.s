.syntax unified
.thumb

vmaxnmv.f64 r0, q1
vmaxnmv.i16 r0, q1
vminnmv.f64 r0, q1
vminnmv.i16 r0, q1
vmaxnmav.f64 r0, q1
vmaxnmav.i16 r0, q1
vminnmav.f64 r0, q1
vminnmav.i16 r0, q1
vmaxnmv.f16 sp, q1
vmaxnmav.f32 pc, q1
vminnmav.f16 sp, q1
vminnmv.f32 pc, q1

.irp op, vmaxnmv, vminnmv, vmaxnmav, vminnmav
.irp cond, eq, ne, gt, ge, lt, le

it \cond
\op\().f16 r0, q1

.endr
.endr

it eq
vmaxnmveq.f32 r0, q1
vmaxnmveq.f32 r0, q1
vpst
vmaxnmveq.f32 r0, q1
vmaxnmvt.f32 r0, q1
vpst
vmaxnmv.f32 r0, q1
it eq
vmaxnmaveq.f32 r0, q1
vmaxnmaveq.f32 r0, q1
vpst
vmaxnmaveq.f32 r0, q1
vmaxnmavt.f32 r0, q1
vpst
vmaxnmav.f32 r0, q1
it eq
vminnmveq.f32 r0, q1
vminnmveq.f32 r0, q1
vpst
vminnmveq.f32 r0, q1
vminnmvt.f32 r0, q1
vpst
vminnmv.f32 r0, q1
it eq
vminnmaveq.f32 r0, q1
vminnmaveq.f32 r0, q1
vpst
vminnmaveq.f32 r0, q1
vminnmavt.f32 r0, q1
vpst
vminnmav.f32 r0, q1
