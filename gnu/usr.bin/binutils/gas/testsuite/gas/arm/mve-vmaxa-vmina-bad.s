.syntax unified
.thumb

vmaxa.u8 q0, q1
vmaxa.s64 q0, q1
vmaxa.f16 q0, q1
vmina.u8 q0, q1
vmina.s64 q0, q1
vmina.f16 q0, q1

.irp op, vmaxa, vmina
.irp cond, eq, ne, gt, ge, lt, le
it \cond
\op\().s8 q0, q1
.endr
.endr

it eq
vmaxaeq.s8 q0, q1
vmaxaeq.s8 q0, q1
vpst
vmaxaeq.s8 q0, q1
vmaxat.s8 q0, q1
vpst
vmaxa.s8 q0, q1
it eq
vminaeq.s8 q0, q1
vminaeq.s8 q0, q1
vpst
vminaeq.s8 q0, q1
vminat.s8 q0, q1
vpst
vmina.s8 q0, q1
