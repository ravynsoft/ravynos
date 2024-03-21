.syntax unified
.thumb
vsri.64 q0, q1, #1
vsri.8 q0, q1, #0
vsri.8 q0, q1, #9
vsri.16 q0, q1, #0
vsri.16 q0, q1, #17
vsri.32 q0, q1, #0
vsri.32 q0, q1, #33

.irp cond, eq, ne, gt, ge, lt, le

it \cond
vsri.16 q0, q1, #4

.endr

it eq
vsrieq.8 q0, q1, #2
vsrieq.8 q0, q1, #2
vpst
vsrieq.8 q0, q1, #2
vsrit.8 q0, q1, #2
vpst
vsri.8 q0, q1, #2

