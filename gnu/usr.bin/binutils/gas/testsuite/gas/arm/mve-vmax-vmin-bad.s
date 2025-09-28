.syntax unified
.thumb

vmax.s64 q0, q1, q2
vmax.f16 q0, q1, q2
vmax.u64 q0, q1, q2
vmax.f32 q0, q1, q2

.irp op, vmax, vmin
.irp cond, eq, ne, gt, ge, lt, le
it \cond
\op\().s8 q0, q1, q2
.endr
.endr

it eq
vmaxeq.s16 q0, q1, q2
vmaxeq.s16 q0, q1, q2
vpst
vmaxeq.s16 q0, q1, q2
vmaxt.s16 q0, q1, q2
vpst
vmax.s16 q0, q1, q2
it eq
vmineq.u32 q0, q1, q2
vmineq.u32 q0, q1, q2
vpst
vmineq.u32 q0, q1, q2
vmint.u32 q0, q1, q2
vpst
vmin.u32 q0, q1, q2
