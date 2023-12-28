.syntax unified
.thumb
vqdmladh.u32 q0, q1, q2
vqdmladh.s64 q0, q1, q2
vqdmladhx.u32 q0, q1, q2
vqdmladhx.s64 q0, q1, q2
vqrdmladh.u32 q0, q1, q2
vqrdmladh.s64 q0, q1, q2
vqrdmladhx.u32 q0, q1, q2
vqrdmladhx.s64 q0, q1, q2

.irp op, vqdmladh, vqdmladhx, vqrdmladh, vqrdmladhx
.irp cond, eq, ne, gt, ge, lt, le

it \cond
\op\().s16 q0, q1, q2

.endr
.endr


it eq
vqdmladheq.s32 q0, q1, q2
vqdmladheq.s32 q0, q1, q2
vpst
vqdmladheq.s32 q0, q1, q2
vqdmladht.s32 q0, q1, q2
vpst
vqdmladh.s32 q0, q1, q2
it eq
vqdmladhxeq.s32 q0, q1, q2
vqdmladhxeq.s32 q0, q1, q2
vpst
vqdmladhxeq.s32 q0, q1, q2
vqdmladhxt.s32 q0, q1, q2
vpst
vqdmladhx.s32 q0, q1, q2
it eq
vqrdmladheq.s32 q0, q1, q2
vqrdmladheq.s32 q0, q1, q2
vpst
vqrdmladheq.s32 q0, q1, q2
vqrdmladht.s32 q0, q1, q2
vpst
vqrdmladh.s32 q0, q1, q2
it eq
vqrdmladhxeq.s32 q0, q1, q2
vqrdmladhxeq.s32 q0, q1, q2
vpst
vqrdmladhxeq.s32 q0, q1, q2
vqrdmladhxt.s32 q0, q1, q2
vpst
vqrdmladhx.s32 q0, q1, q2
