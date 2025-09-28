.macro cond type, lastreg
.irp cond, eq, ne, gt, ge, lt, le
it \cond
vqshl.\type q0, q0, \lastreg
.endr
.endm

.syntax unified
.thumb
vqshl.s64 q0, q0, #0
vqshl.i32 q0, q0, #0
vqshl.s8 q0, q1, #8
vqshl.u16 q0, q1, #16
vqshl.s32 q0, q1, #32
vqshl.s64 q0, r1
vqshl.i16 q0, r1
vqshl.u16 q0, sp
vqshl.s32 q0, pc
vqshl.s64 q0, q1, q2
vqshl.i32 q0, q1, q2
cond u32, #0
cond s8, r1
cond s16, q2
it eq
vqshleq.s16 q0, q1, #0
vqshleq.s16 q0, q1, #0
vpst
vqshleq.s16 q0, q1, #0
vqshlt.s16 q0, q1, #0
vpst
vqshl.s16 q0, q1, #0
it eq
vqshleq.s16 q0, r1
vqshleq.s16 q0, r1
vpst
vqshleq.s16 q0, r1
vqshlt.s16 q0, r1
vpst
vqshl.s16 q0, r1
it eq
vqshleq.s16 q0, q1, q2
vqshleq.s16 q0, q1, q2
vpst
vqshleq.s16 q0, q1, q2
vqshlt.s16 q0, q1, q2
vpst
vqshl.s16 q0, q1, q2

