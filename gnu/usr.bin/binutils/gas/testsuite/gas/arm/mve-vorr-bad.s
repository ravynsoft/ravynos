.syntax unified
.thumb

.irp cond, eq, ne, gt, ge, lt, le

it \cond
vorr q0, q1, q2

.endr

it eq
vorreq q0, q1, q2
vorreq q0, q1, q2
vpst
vorreq q0, q1, q2
vpst
vorr q0, q1, q2
vorrt q0, q1, q2

.irp cond, eq, ne, gt, ge, lt, le

it \cond
vorr.i16 q0, #255

.endr

it eq
vorreq.i16 q0, #255
vorreq.i16 q0, #255
vpst
vorreq.i16 q0, #255
vpst
vorr.i16 q0, #255
vorrt.i16 q0, #255
vorr.i8 q0, #255
vorr.i64 q0, #255
vorr.i16 q0, #257
vorr.i32 q0, #257
