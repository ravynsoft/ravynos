.syntax unified
.thumb
vhcadd.u8 q0, q1, q2, #90
vhcadd.i8 q0, q1, q2, #90
vhcadd.s64 q0, q1, q2, #90
vhcadd.s8 q0, q1, q2, #0
vhcadd.s8 q0, q1, q2, #180

.irp cond, eq, ne, gt, ge, lt, le

it \cond
vhcadd.s8 q0, q1, q2, #90

.endr

it eq
vhcaddeq.s8 q0, q1, q2, #90
vhcaddeq.s8 q0, q1, q2, #90
vpst
vhcaddeq.s8 q0, q1, q2, #90
vhcaddt.s8 q0, q1, q2, #90
vpst
vhcadd.s8 q0, q1, q2, #90
