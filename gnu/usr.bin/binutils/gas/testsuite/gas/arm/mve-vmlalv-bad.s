.syntax unified
.thumb
.irp op, vmlalv, vmlalva
.irp cond, eq, ne, gt, ge, lt, le

it \cond
\op\().s16 r0, r1, q1, q2

.endr
.endr

vmlalv.s64 r0, r1, q1, q2
vmlalv.f32 r0, r1, q1, q2
vmlalv.s8 r0, r1, q1, q2
vmlalv.s16 r0, q1, q2
vmlalva.s64 r0, r1, q1, q2
vmlalva.f32 r0, r1, q1, q2
vmlalva.s8 r0, r1, q1, q2
vmlalva.s16 r0, q1, q2
vmlalvx.s16 r0, r1, q1, q2
vmlalvax.s16 r0, r1, q1, q2
it eq
vmlalveq.s16 r0, r1, q1, q2
vmlalveq.s16 r0, r1, q1, q2
vmlalveq.s16 r0, r1, q1, q2
vmlalvt.s16 r0, r1, q1, q2
vpst
vmlalv.s16 r0, r1, q1, q2
it eq
vmlalvaeq.s16 r0, r1, q1, q2
vmlalvaeq.s16 r0, r1, q1, q2
vmlalvaeq.s16 r0, r1, q1, q2
vmlalvat.s16 r0, r1, q1, q2
vpst
vmlalva.s16 r0, r1, q1, q2
