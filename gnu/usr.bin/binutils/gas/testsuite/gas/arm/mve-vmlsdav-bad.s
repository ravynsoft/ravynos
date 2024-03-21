.syntax unified
.thumb

vmlsdav.s16 r1, q1, q2
vmlsdav.u16 r0, q1, q2

.irp op, vmlsdav, vmlsdava, vmlsdavx, vmlsdavax
.irp cond, eq, ne, gt, ge, lt, le

it \cond
\op\().s16 r0, q1, q2

.endr
.endr

it eq
vmlsdaveq.s16 r0, q1, q2
vmlsdaveq.s16 r0, q1, q2
vpst
vmlsdaveq.s16 r0, q1, q2
vmlsdavt.s16 r0, q1, q2
vpst
vmlsdav.s16 r0, q1, q2
it eq
vmlsdavaeq.s16 r0, q1, q2
vmlsdavaeq.s16 r0, q1, q2
vpst
vmlsdavaeq.s16 r0, q1, q2
vmlsdavat.s16 r0, q1, q2
vpst
vmlsdava.s16 r0, q1, q2
it eq
vmlsdavxeq.s16 r0, q1, q2
vmlsdavxeq.s16 r0, q1, q2
vpst
vmlsdavxeq.s16 r0, q1, q2
vmlsdavxt.s16 r0, q1, q2
vpst
vmlsdavx.s16 r0, q1, q2
it eq
vmlsdavaxeq.s16 r0, q1, q2
vmlsdavaxeq.s16 r0, q1, q2
vpst
vmlsdavaxeq.s16 r0, q1, q2
vmlsdavaxt.s16 r0, q1, q2
vpst
vmlsdavax.s16 r0, q1, q2
