.syntax unified
.thumb

.irp op, vmladav, vmladava, vmladavx, vmladavax
.irp cond, eq, ne, gt, ge, lt, le

it \cond
\op\().s16 r0, q1, q2

.endr
.endr


vmladav.s64 r0, q1, q2
vmladav.f32 r0, q1, q2
vmladava.s64 r0, q1, q2
vmladava.f32 r0, q1, q2
vmladavx.s64 r0, q1, q2
vmladavx.f32 r0, q1, q2
vmladavax.s64 r0, q1, q2
vmladavax.f32 r0, q1, q2
vmladavx.u32 r0, q1, q2
vmladavax.u16 r0, q1, q2
it eq
vmladaveq.s32 r0, q1, q2
vmladaveq.s32 r0, q1, q2
vpst
vmladaveq.s32 r0, q1, q2
vmladavt.s32 r0, q1, q2
vpst
vmladav.s32 r0, q1, q2
it eq
vmladavaeq.s32 r0, q1, q2
vmladavaeq.s32 r0, q1, q2
vpst
vmladavaeq.s32 r0, q1, q2
vmladavat.s32 r0, q1, q2
vpst
vmladava.s32 r0, q1, q2
it eq
vmladavxeq.s32 r0, q1, q2
vmladavxeq.s32 r0, q1, q2
vpst
vmladavxeq.s32 r0, q1, q2
vmladavxt.s32 r0, q1, q2
vpst
vmladavx.s32 r0, q1, q2
it eq
vmladavaxeq.s32 r0, q1, q2
vmladavaxeq.s32 r0, q1, q2
vpst
vmladavaxeq.s32 r0, q1, q2
vmladavaxt.s32 r0, q1, q2
vpst
vmladavax.s32 r0, q1, q2
