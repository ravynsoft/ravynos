.macro cond type, lastreg
.irp cond, eq, ne, gt, ge, lt, le
it \cond
vshl.\type q0, q0, \lastreg
.endr
.endm

.syntax unified
.thumb
vshl.i64 q0, q0, #0
vshl.i8 q0, q1, #8
vshl.i16 q0, q1, #16
vshl.i32 q0, q1, #32
vshl.s64 q0, r1
vshl.i16 q0, r1
vshl.u16 q0, sp
vshl.s32 q0, pc
vshl.s64 q0, q1, q2
vshl.i32 q0, q1, q2
cond i32, #0
cond s8, r1
cond s16, q2
it eq
vshleq.i16 q0, q1, #0
vshleq.i16 q0, q1, #0
vpst
vshleq.i16 q0, q1, #0
vshlt.i16 q0, q1, #0
vpst
vshl.i16 q0, q1, #0
it eq
vshleq.s16 q0, r1
vshleq.s16 q0, r1
vpst
vshleq.s16 q0, r1
vshlt.s16 q0, r1
vpst
vshl.s16 q0, r1
it eq
vshleq.s16 q0, q1, q2
vshleq.s16 q0, q1, q2
vpst
vshleq.s16 q0, q1, q2
vshlt.s16 q0, q1, q2
vpst
vshl.s16 q0, q1, q2
