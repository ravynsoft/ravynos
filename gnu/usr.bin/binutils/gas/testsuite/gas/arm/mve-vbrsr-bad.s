.syntax unified
.thumb

.irp cond, eq, ne, gt, ge, lt, le

it \cond
vbrsr.16 q0, q1, r2

.endr

vbrsr.64 q0, q1, r2
vbrsr.32 q0, q1, q2
it eq
vbrsreq.32 q0, q1, r2
vbrsreq.32 q0, q1, r2
vpst
vbrsreq.32 q0, q1, r2
vpst
vbrsr.32 q0, q1, r2
vbrsrt.32 q0, q1, r2
