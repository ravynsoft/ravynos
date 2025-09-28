.syntax unified
.thumb

.irp cond, eq, ne, gt, ge, lt, le

it \cond
veor q0, q1, q2

.endr

it eq
veoreq q0, q1, q2
veoreq q0, q1, q2
vpst
veoreq q0, q1, q2
vpst
veor q0, q1, q2
veort q0, q1, q2
