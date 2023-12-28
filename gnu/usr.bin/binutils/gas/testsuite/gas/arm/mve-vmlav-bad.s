.syntax unified
.thumb
.irp op, vmlav, vmlava
.irp cond, eq, ne, gt, ge, lt, le

it \cond
\op\().s16 r0, q1, q2

.endr
.endr

vmlav.s64 r0, q1, q2
vmlav.f32 r0, q1, q2
vmlava.s64 r0, q1, q2
vmlava.f32 r0, q1, q2
vmlavx.s32 r0, q1, q2
vmlavax.s32 r0, q1, q2
it eq
vmlaveq.s32 r0, q1, q2
vmlaveq.s32 r0, q1, q2
vpst
vmlaveq.s32 r0, q1, q2
vmlavt.s32 r0, q1, q2
vpst
vmlav.s32 r0, q1, q2
it eq
vmlavaeq.s32 r0, q1, q2
vmlavaeq.s32 r0, q1, q2
vpst
vmlavaeq.s32 r0, q1, q2
vmlavat.s32 r0, q1, q2
vpst
vmlava.s32 r0, q1, q2
