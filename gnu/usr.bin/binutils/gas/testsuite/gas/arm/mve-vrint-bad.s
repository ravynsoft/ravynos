.syntax unified
.thumb
.irp mode, n, x, a, z, m, p
vrint\mode\().i16 q0, q1
vrint\mode\().f64 q0, q1
.endr
vrintr.f16 q0, q1

.irp mode, n, x, a, z, m, p
.irp cond, eq, ne, gt, ge, lt, le

it \cond
vrint\mode\().f16 q0, q1

.endr

it eq
vrint\mode\()eq.f16 q0, q1
vrint\mode\()eq.f16 q0, q1
vpst
vrint\mode\()eq.f16 q0, q1
vrint\mode\()t.f16 q0, q1
vpst
vrint\mode\().f16 q0, q1
.endr
