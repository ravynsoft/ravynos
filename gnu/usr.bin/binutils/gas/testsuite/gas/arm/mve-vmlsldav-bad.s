.syntax unified
.thumb

vmlsldav.s16 r0, sp, q1, q2
vmlsldav.u16 r0, r1, q1, q2

.irp op, vmlsldav, vmlsldava, vmlsldavx, vmlsldavax
.irp cond, eq, ne, gt, ge, lt, le

it \cond
\op\().s16 r0, r1, q1, q2

.endr
.endr

vmlsldav.s64 r0, r1, q1, q2
vmlsldav.f32 r0, r1, q1, q2
vmlsldav.s8 r0, r1, q1, q2
vmlsldav.s16 r0, q1, q2
vmlsldava.s64 r0, r1, q1, q2
vmlsldava.f32 r0, r1, q1, q2
vmlsldava.s8 r0, r1, q1, q2
vmlsldava.s16 r0, q1, q2
vmlsldavx.s64 r0, r1, q1, q2
vmlsldavx.f32 r0, r1, q1, q2
vmlsldavx.s8 r0, r1, q1, q2
vmlsldavx.s16 r0, q1, q2
vmlsldavax.s64 r0, r1, q1, q2
vmlsldavax.f32 r0, r1, q1, q2
vmlsldavax.s8 r0, r1, q1, q2
vmlsldavax.s16 r0, q1, q2
it eq
vmlsldaveq.s16 r0, r1, q1, q2
vmlsldaveq.s16 r0, r1, q1, q2
vmlsldaveq.s16 r0, r1, q1, q2
vmlsldavt.s16 r0, r1, q1, q2
vpst
vmlsldav.s16 r0, r1, q1, q2
it eq
vmlsldavaeq.s16 r0, r1, q1, q2
vmlsldavaeq.s16 r0, r1, q1, q2
vmlsldavaeq.s16 r0, r1, q1, q2
vmlsldavat.s16 r0, r1, q1, q2
vpst
vmlsldava.s16 r0, r1, q1, q2
it eq
vmlsldavxeq.s16 r0, r1, q1, q2
vmlsldavxeq.s16 r0, r1, q1, q2
vmlsldavxeq.s16 r0, r1, q1, q2
vmlsldavxt.s16 r0, r1, q1, q2
vpst
vmlsldavx.s16 r0, r1, q1, q2
it eq
vmlsldavaxeq.s16 r0, r1, q1, q2
vmlsldavaxeq.s16 r0, r1, q1, q2
vmlsldavaxeq.s16 r0, r1, q1, q2
vmlsldavaxt.s16 r0, r1, q1, q2
vpst
vmlsldavax.s16 r0, r1, q1, q2

