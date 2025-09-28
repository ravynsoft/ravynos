.syntax unified
.thumb
vfma.f32 q0, q1, sp
vfma.f32 q0, q1, pc
vfma.f64 q0, q1, q2
vfma.32 q0, q1, q2
vfms.f64 q0, q1, q2
vfms.32 q0, q1, q2
vfma.f64 d0, d1, d2

.irp cond, eq, ne, gt, ge, lt, le
it \cond
vfma.f32 q0, q1, q2
.endr

.irp cond, eq, ne, gt, ge, lt, le
it \cond
vfma.f32 q0, q1, r2
.endr

.irp cond, eq, ne, gt, ge, lt, le
it \cond
vfms.f32 q0, q1, q2
.endr

it eq
vfmaeq.f16 q0, q1, q2
vfmaeq.f16 q0, q1, q2
vpst
vfmaeq.f16 q0, q1, q2
vfmat.f16 q0, q1, q2
vpst
vfma.f16 q0, q1, q2
it eq
vfmseq.f16 q0, q1, q2
vfmseq.f16 q0, q1, q2
vpst
vfmseq.f16 q0, q1, q2
vfmst.f16 q0, q1, q2
vpst
vfms.f16 q0, q1, q2
