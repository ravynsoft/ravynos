.syntax unified
.thumb
vsli.64 q0, q1, #1
vsli.8 q0, q1, #8
vsli.16 q0, q1, #16
vsli.32 q0, q1, #32

.irp cond, eq, ne, gt, ge, lt, le

it \cond
vsli.16 q0, q1, #4

.endr

it eq
vslieq.8 q0, q1, #2
vslieq.8 q0, q1, #2
vpst
vslieq.8 q0, q1, #2
vslit.8 q0, q1, #2
vpst
vsli.8 q0, q1, #2
