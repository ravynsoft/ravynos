.syntax unified
.thumb

.irp cond, eq, ne, gt, ge, lt, le
it \cond
vand q0, q1, q2
.endr

it eq
vandeq q0, q1, q2
vandeq q0, q1, q2
vpst
vandeq q0, q1, q2
vpst
vand q0, q1, q2
vandt q0, q1, q2

.irp cond, eq, ne, gt, ge, lt, le
it \cond
vand.i16 q0, #255
.endr

it eq
vandeq.i16 q0, #255
vandeq.i16 q0, #255
vpst
vandeq.i16 q0, #255
vpst
vand.i16 q0, #255
vandt.i16 q0, #255
vand.i8 q0, #255
vand.i64 q0, #255
vand.i16 q0, #0
vand.i32 q0, #0
