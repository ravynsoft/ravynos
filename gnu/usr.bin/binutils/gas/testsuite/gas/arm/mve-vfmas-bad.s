.syntax unified
.thumb
vfmas.f32 q0, q1, sp
vfmas.f32 q0, q1, pc
vfmas.i32 q0, q1, r2
vfmas.f64 q0, q1, r2

.irp cond, eq, ne, gt, ge, lt, le

it \cond
vfmas.f32 q0, q1, r2

.endr

it eq
vfmaseq.f32 q0, q1, r2
vfmaseq.f32 q0, q1, r2
vpst
vfmaseq.f32 q0, q1, r2
vfmast.f32 q0, q1, r2
vpst
vfmas.f32 q0, q1, r2
