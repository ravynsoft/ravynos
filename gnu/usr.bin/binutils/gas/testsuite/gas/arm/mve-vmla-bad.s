.syntax unified
.thumb
vmla.f16 q0, q1, r2
vmla.s64 q0, q1, r2
vmla.s32 q0, q1, q2
vmla.s32 q0, q1, sp
vmla.s32 q0, q1, pc

.irp cond, eq, ne, gt, ge, lt, le

it \cond
vmla.s16 q0, q1, r2

.endr

it eq
vmlaeq.u16 q0, q1, r2
vmlaeq.u16 q0, q1, r2
vpst
vmlaeq.u16 q0, q1, r2
vmlat.u16 q0, q1, r2
vpst
vmla.u16 q0, q1, r2
