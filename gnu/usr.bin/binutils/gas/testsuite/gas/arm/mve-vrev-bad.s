.syntax unified
.thumb
vrev16.16 q0, q1
vrev32.32 q0, q1
vrev64.64 q0, q1
vrev64.8  q0, q0

.irp op, vrev16, vrev32, vrev64
.irp cond, eq, ne, gt, ge, lt, le

it \cond
\op\().8 q0, q1

.endr
.endr

it eq
vrev16eq.8 q0, q1
vrev16eq.8 q0, q1
vpst
vrev16eq.8 q0, q1
vrev16t.8 q0, q1
vpst
vrev16.8 q0, q1
it eq
vrev32eq.8 q0, q1
vrev32eq.8 q0, q1
vpst
vrev32eq.8 q0, q1
vrev32t.8 q0, q1
vpst
vrev32.8 q0, q1
it eq
vrev64eq.8 q0, q1
vrev64eq.8 q0, q1
vpst
vrev64eq.8 q0, q1
vrev64t.8 q0, q1
vpst
vrev64.8 q0, q1
