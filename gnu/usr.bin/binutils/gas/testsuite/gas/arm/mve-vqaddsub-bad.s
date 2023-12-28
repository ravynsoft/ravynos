.macro cond op, lastop
.irp cond, eq, ne, gt, ge, lt, le
it \cond
\op\().s16 q0, q1, \lastop
.endr
.endm

.syntax unified
.thumb
vqadd.s64 q0, q1, q2
vqsub.u64 q0, q1, q2
vqadd.s64 q0, q1, r2
vqsub.s64 q0, q1, r2
vqadd.f32 q0, q1, q2
vqsub.f32 q0, q1, q2
vqadd.f32 q0, q1, r2
vqsub.f32 q0, q1, r2
vqadd.s16 q0, q1, sp
vqadd.s16 q0, q1, pc
vqsub.s16 q0, q1, sp
vqsub.s16 q0, q1, pc
cond vqadd q2
cond vqadd r2
cond vqsub q2
cond vqsub r2
it eq
vqaddeq.s32 q0, q1, q2
vqaddeq.s32 q0, q1, q2
vpst
vqaddeq.s32 q0, q1, q2
vqaddt.s32 q0, q1, q2
vpst
vqadd.s32 q0, q1, q2
it eq
vqsubeq.s32 q0, q1, q2
vqsubeq.s32 q0, q1, q2
vpst
vqsubeq.s32 q0, q1, q2
vqsubt.s32 q0, q1, q2
vpst
vqsub.s32 q0, q1, q2
it eq
vqaddeq.s32 q0, q1, r2
vqaddeq.s32 q0, q1, r2
vpst
vqaddeq.s32 q0, q1, r2
vqaddt.s32 q0, q1, r2
vpst
vqadd.s32 q0, q1, r2
it eq
vqsubeq.s32 q0, q1, r2
vqsubeq.s32 q0, q1, r2
vpst
vqsubeq.s32 q0, q1, r2
vqsubt.s32 q0, q1, r2
vpst
vqsub.s32 q0, q1, r2
