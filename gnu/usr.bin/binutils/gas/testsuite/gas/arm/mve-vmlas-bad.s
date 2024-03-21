.syntax unified
.thumb
vmlas.s64 q0, q1, r2
vmlas.f32 q0, q1, r2
vmlas.u32 q0, q1, sp
vmlas.u32 q0, q1, pc

.irp cond, eq, ne, gt, ge, lt, le

it \cond
vmlas.s16 q0, q1, r2

.endr

it eq
vmlaseq.s16 q0, q1, r2
vmlaseq.s16 q0, q1, r2
vpst
vmlaseq.s16 q0, q1, r2
vmlast.s16 q0, q1, r2
vpst
vmlas.s16 q0, q1, r2
