.syntax unified
.thumb
vdup.f16 q0, r1
vdup.64 q0, r1
vdup.32 q0, d0[1]
vdup.32 q0, sp
vdup.32 q0, pc

.irp cond, eq, ne, gt, ge, lt, le

it \cond
vdup.32 q0, r2

.endr

it eq
vdupeq.32 q0, r2
vdupeq.32 q0, r2
vpste
vdupeq.32 q0, r2
vdupt.32 q0, r2
vpst
vdup.32 q0, r2
