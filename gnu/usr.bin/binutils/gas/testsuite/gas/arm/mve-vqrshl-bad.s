.syntax unified
.thumb
vqrshl.s64 q0, q1, q2
vqrshl.u64 q0, q1, q2
vqrshl.i32 q0, q1, q2
vqrshl.s64 q0, r2
vqrshl.u64 q0, r2
vqrshl.i32 q0, r2
vqrshl.s32 q0, q1, r2
vqrshl.s32 q0, pc
vqrshl.s32 q0, sp

.irp lastreg, q2, r2
.irp cond, eq, ne, gt, ge, lt, le
it \cond
vqrshl.s16 q0, q0, \lastreg
.endr
.endr


it eq
vqrshleq.s32 q0, q1, q2
vqrshleq.s32 q0, q1, q2
vpst
vqrshleq.s32 q0, q1, q2
vqrshlt.s32 q0, q1, q2
vpst
vqrshl.s32 q0, q1, q2
it eq
vqrshleq.s32 q0, r2
vqrshleq.s32 q0, r2
vpst
vqrshleq.s32 q0, r2
vqrshlt.s32 q0, r2
vpst
vqrshl.s32 q0, r2
