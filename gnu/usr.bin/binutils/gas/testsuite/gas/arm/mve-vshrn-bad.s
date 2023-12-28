.syntax unified
.thumb

vshrnt.i64 q0, q1, #1
vshrnb.i64 q0, q1, #1
vrshrnt.i64 q0, q1, #1
vrshrnb.i64 q0, q1, #1
vshrnt.i16 q0, q1, #0
vshrnt.i16 q0, q1, #9
vshrnt.i32 q0, q1, #0
vshrnt.i32 q0, q1, #17
vshrnb.i16 q0, q1, #0
vshrnb.i16 q0, q1, #9
vshrnb.i32 q0, q1, #0
vshrnb.i32 q0, q1, #17

.irp op, vshrnt, vshrnb, vrshrnt, vrshrnb
.irp cond, eq, ne, gt, ge, lt, le

it \cond
\op\().i32 q0, q1, #1

.endr
.endr

it eq
vshrnteq.i32 q0, q1, #1
vshrnteq.i32 q0, q1, #1
vpst
vshrnteq.i32 q0, q1, #1
vshrntt.i32 q0, q1, #1
vpst
vshrnt.i32 q0, q1, #1
it eq
vshrnbeq.i32 q0, q1, #1
vshrnbeq.i32 q0, q1, #1
vpst
vshrnbeq.i32 q0, q1, #1
vshrnbt.i32 q0, q1, #1
vpst
vshrnb.i32 q0, q1, #1
it eq
vrshrnteq.i32 q0, q1, #1
vrshrnteq.i32 q0, q1, #1
vpst
vrshrnteq.i32 q0, q1, #1
vrshrntt.i32 q0, q1, #1
vpst
vrshrnt.i32 q0, q1, #1
it eq
vrshrnbeq.i32 q0, q1, #1
vrshrnbeq.i32 q0, q1, #1
vpst
vrshrnbeq.i32 q0, q1, #1
vrshrnbt.i32 q0, q1, #1
vpst
vrshrnb.i32 q0, q1, #1
