.syntax unified
.thumb

.irp cond, eq, ne, gt, ge, lt, le
.irp size, .
it \cond
vcmp.f32 gt, q0, q1
.endr
.endr

.irp cond, eq, ne, gt, ge, lt, le
.irp size, .
it \cond
vcmp.f16 eq, q0, r1
.endr
.endr

vcmp.f64 eq, q0, q1
vcmp.f32 eq, q0, sp
it eq
vcmpeq.f32 eq, q0, q1
vcmpeq.f32 eq, q0, q1
vpst
vcmpeq.f32 eq, q0, q1
vcmpt.f32 eq, q0, q1
vpst
vcmp.f32 eq, q0, q1
it eq
vcmpeq.f32 eq, q0, r1
vcmpeq.f32 eq, q0, r1
vpst
vcmpeq.f32 eq, q0, r1
vcmpt.f32 eq, q0, r1
vpst
vcmp.f32 eq, q0, r1
