.syntax unified
.thumb
vqdmlsdh.u32 q0, q1, q2
vqdmlsdh.s64 q0, q1, q2
vqdmlsdhx.u32 q0, q1, q2
vqdmlsdhx.s64 q0, q1, q2
vqrdmlsdh.u32 q0, q1, q2
vqrdmlsdh.s64 q0, q1, q2
vqrdmlsdhx.u32 q0, q1, q2
vqrdmlsdhx.s64 q0, q1, q2

.irp op, vqdmlsdh, vqdmlsdhx, vqrdmlsdh, vqrdmlsdhx
.irp cond, eq, ne, gt, ge, lt, le

it \cond
\op\().s16 q0, q1, q2

.endr
.endr


it eq
vqdmlsdheq.s32 q0, q1, q2
vqdmlsdheq.s32 q0, q1, q2
vpst
vqdmlsdheq.s32 q0, q1, q2
vqdmlsdht.s32 q0, q1, q2
vpst
vqdmlsdh.s32 q0, q1, q2
it eq
vqdmlsdhxeq.s32 q0, q1, q2
vqdmlsdhxeq.s32 q0, q1, q2
vpst
vqdmlsdhxeq.s32 q0, q1, q2
vqdmlsdhxt.s32 q0, q1, q2
vpst
vqdmlsdhx.s32 q0, q1, q2
it eq
vqrdmlsdheq.s32 q0, q1, q2
vqrdmlsdheq.s32 q0, q1, q2
vpst
vqrdmlsdheq.s32 q0, q1, q2
vqrdmlsdht.s32 q0, q1, q2
vpst
vqrdmlsdh.s32 q0, q1, q2
it eq
vqrdmlsdhxeq.s32 q0, q1, q2
vqrdmlsdhxeq.s32 q0, q1, q2
vpst
vqrdmlsdhxeq.s32 q0, q1, q2
vqrdmlsdhxt.s32 q0, q1, q2
vpst
vqrdmlsdhx.s32 q0, q1, q2
