.macro cond, op, lastreg
.irp cond, eq, ne, gt, ge, lt, le
it \cond
\op\().s8 q0, q1, \lastreg
.endr
.endm

.syntax unified
.thumb
vhadd.i8 q0, q1, q2
vhadd.s64 q0, q1, q2
vhadd.i8 q0, q1, r2
vhadd.s64 q0, q1, r2
vhsub.i16 q0, q1, q2
vhsub.u64 q0, q1, q2
vhsub.i16 q0, q1, r2
vhsub.u64 q0, q1, r2
vrhadd.i32 q0, q1, q2
vrhadd.s64 q0, q1, q2
vhadd.s8 q0, q1, sp
vhadd.s8 q0, q1, pc
vhsub.s8 q0, q1, sp
vhsub.s8 q0, q1, pc
vrhadd.s8 q0, q1, r2
cond vhadd, r2
cond vhadd, q2
cond vhsub, r2
cond vhsub, q2
cond vrhadd, q2
it eq
vhaddeq.s8 q0, q1, r2
vhaddeq.s8 q0, q1, r2
vpst
vhaddeq.s8 q0, q1, r2
vhaddt.s8 q0, q1, r2
vpst
vhadd.s8 q0, q1, r2
it eq
vhaddeq.s8 q0, q1, q2
vhaddeq.s8 q0, q1, q2
vpst
vhaddeq.s8 q0, q1, q2
vhaddt.s8 q0, q1, q2
vpst
vhadd.s8 q0, q1, q2
it eq
vhsubeq.s8 q0, q1, r2
vhsubeq.s8 q0, q1, r2
vpst
vhsubeq.s8 q0, q1, r2
vhsubt.s8 q0, q1, r2
vpst
vhsub.s8 q0, q1, r2
it eq
vhsubeq.s8 q0, q1, q2
vhsubeq.s8 q0, q1, q2
vpst
vhsubeq.s8 q0, q1, q2
vhsubt.s8 q0, q1, q2
vpst
vhsub.s8 q0, q1, q2
it eq
vrhaddeq.s8 q0, q1, q2
vrhaddeq.s8 q0, q1, q2
vpst
vrhaddeq.s8 q0, q1, q2
vrhaddt.s8 q0, q1, q2
vpst
vrhadd.s8 q0, q1, q2
