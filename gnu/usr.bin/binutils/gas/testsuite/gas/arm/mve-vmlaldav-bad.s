.syntax unified
.thumb

vmlaldav.s16 r0, sp, q1, q2

.irp op, vmlaldav, vmlaldava, vmlaldavx, vmlaldavax
.irp cond, eq, ne, gt, ge, lt, le

it \cond
\op\().s16 r0, r1, q1, q2

.endr
.endr

vmlaldav.s64 r0, r1, q1, q2
vmlaldav.f32 r0, r1, q1, q2
vmlaldav.s8 r0, r1, q1, q2
vmlaldav.s16 r0, q1, q2
vmlaldava.s64 r0, r1, q1, q2
vmlaldava.f32 r0, r1, q1, q2
vmlaldava.s8 r0, r1, q1, q2
vmlaldava.s16 r0, q1, q2
vmlaldavx.s64 r0, r1, q1, q2
vmlaldavx.f32 r0, r1, q1, q2
vmlaldavx.s8 r0, r1, q1, q2
vmlaldavx.s16 r0, q1, q2
vmlaldavax.s64 r0, r1, q1, q2
vmlaldavax.f32 r0, r1, q1, q2
vmlaldavax.s8 r0, r1, q1, q2
vmlaldavax.s16 r0, q1, q2
it eq
vmlaldaveq.s16 r0, r1, q1, q2
vmlaldaveq.s16 r0, r1, q1, q2
vmlaldaveq.s16 r0, r1, q1, q2
vmlaldavt.s16 r0, r1, q1, q2
vpst
vmlaldav.s16 r0, r1, q1, q2
it eq
vmlaldavaeq.s16 r0, r1, q1, q2
vmlaldavaeq.s16 r0, r1, q1, q2
vmlaldavaeq.s16 r0, r1, q1, q2
vmlaldavat.s16 r0, r1, q1, q2
vpst
vmlaldava.s16 r0, r1, q1, q2
it eq
vmlaldavxeq.s16 r0, r1, q1, q2
vmlaldavxeq.s16 r0, r1, q1, q2
vmlaldavxeq.s16 r0, r1, q1, q2
vmlaldavxt.s16 r0, r1, q1, q2
vpst
vmlaldavx.s16 r0, r1, q1, q2
it eq
vmlaldavaxeq.s16 r0, r1, q1, q2
vmlaldavaxeq.s16 r0, r1, q1, q2
vmlaldavaxeq.s16 r0, r1, q1, q2
vmlaldavaxt.s16 r0, r1, q1, q2
vpst
vmlaldavax.s16 r0, r1, q1, q2
