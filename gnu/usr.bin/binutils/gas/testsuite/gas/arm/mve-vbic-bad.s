.syntax unified
.thumb

.irp cond, eq, ne, gt, ge, lt, le
it \cond
vbic q0, q1, q2
.endr

it eq
vbiceq q0, q1, q2
vbiceq q0, q1, q2
vpst
vbiceq q0, q1, q2
vpst
vbic q0, q1, q2
vbict q0, q1, q2

.irp cond, eq, ne, gt, ge, lt, le
it \cond
vbic.i16 q0, #255
.endr

it eq
vbiceq.i16 q0, #255
vbiceq.i16 q0, #255
vpst
vbiceq.i16 q0, #255
vpst
vbic.i16 q0, #255
vbict.i16 q0, #255
vbic.i8 q0, #255
vbic.i64 q0, #255
vbic.i16 q0, #257
vbic.i32 q0, #257
