.syntax unified
.thumb

vsbc.i16 q0, q1, q2
vsbci.i16 q0, q1, q2

.irp op, vsbc, vsbci
.irp cond, eq, ne, gt, ge, lt, le
it \cond
\op\().i32 q0, q1, q2
.endr
.endr

it eq
vsbceq.i32 q0, q1, q2
vsbceq.i32 q0, q1, q2
vpst
vsbceq.i32 q0, q1, q2
vpst
vsbc.i32 q0, q1, q2
vsbct.i32 q0, q1, q2
it eq
vsbcieq.i32 q0, q1, q2
vsbcieq.i32 q0, q1, q2
vpst
vsbcieq.i32 q0, q1, q2
vpst
vsbci.i32 q0, q1, q2
vsbcit.i32 q0, q1, q2
