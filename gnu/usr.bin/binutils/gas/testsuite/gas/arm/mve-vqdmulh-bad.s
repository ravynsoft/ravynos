.macro cond op, lastreg
.irp cond, eq, ne, gt, ge, lt, le
it \cond
\op\().s16 q0, q1, \lastreg
.endr
.endm

.syntax unified
.thumb
vqdmulh.s64 q0, q1, q2
vqdmulh.u8 q0, q1, q2
vqrdmulh.s64 q0, q1, q2
vqrdmulh.u8 q0, q1, q2
vqdmulh.s64 q0, q1, r2
vqdmulh.u8 q0, q1, r2
vqrdmulh.s64 q0, q1, r2
vqrdmulh.u8 q0, q1, r2
vqdmulh.s8 q0, q1, sp
vqdmulh.s8 q0, q1, pc
vqrdmulh.s8 q0, q1, sp
vqrdmulh.s8 q0, q1, pc
cond vqdmulh, q2
cond vqrdmulh, q2
cond vqdmulh, r2
cond vqrdmulh, r2
it eq
vqdmulheq.s8 q0, q1, q2
vqdmulheq.s8 q0, q1, q2
vpst
vqdmulheq.s8 q0, q1, q2
vqdmulht.s8 q0, q1, q2
vpst
vqdmulh.s8 q0, q1, q2
it eq
vqrdmulheq.s8 q0, q1, q2
vqrdmulheq.s8 q0, q1, q2
vpst
vqrdmulheq.s8 q0, q1, q2
vqrdmulht.s8 q0, q1, q2
vpst
vqrdmulh.s8 q0, q1, q2
it eq
vqdmulheq.s8 q0, q1, r2
vqdmulheq.s8 q0, q1, r2
vpst
vqdmulheq.s8 q0, q1, r2
vqdmulht.s8 q0, q1, r2
vpst
vqdmulh.s8 q0, q1, r2
it eq
vqrdmulheq.s8 q0, q1, r2
vqrdmulheq.s8 q0, q1, r2
vpst
vqrdmulheq.s8 q0, q1, r2
vqrdmulht.s8 q0, q1, r2
vpst
vqrdmulh.s8 q0, q1, r2
