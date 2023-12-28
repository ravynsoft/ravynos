.syntax unified
.thumb

vrshl.i16 q0, q1, q2
vrshl.i16 q0, r2
vrshl.s64 q0, q1, q2
vrshl.s64 q0, r2
vrshl.s32 q0, sp
vrshl.s32 q0, pc

.irp lastreg, q2, r2
.irp cond, eq, ne, gt, ge, lt, le
it \cond
vrshl.s32 q0, q0, \lastreg
.endr
.endr

it eq
vrshleq.s32 q0, q1, q2
vrshleq.s32 q0, q1, q2
vpst
vrshleq.s32 q0, q1, q2
vrshlt.s32 q0, q1, q2
vpst
vrshl.s32 q0, q1, q2
it eq
vrshleq.s32 q0, r2
vrshleq.s32 q0, r2
vpst
vrshleq.s32 q0, r2
vrshlt.s32 q0, r2
vpst
vrshl.s32 q0, r2
