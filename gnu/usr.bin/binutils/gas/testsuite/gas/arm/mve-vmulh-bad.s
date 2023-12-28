.syntax unified
.thumb
vmulh.f16 q0, q1, q2
vmulh.i32 q0, q1, q2
vmulh.s64 q0, q1, q2
vrmulh.f16 q0, q1, q2
vrmulh.i32 q0, q1, q2
vrmulh.s64 q0, q1, q2

.irp op, vmulh, vrmulh
.irp cond, eq, ne, gt, ge, lt, le
it \cond
\op\().u16 q0, q1, q2
.endr
.endr


it eq
vmulheq.s16 q0, q1, q2
vmulheq.s16 q0, q1, q2
vpst
vmulheq.s16 q0, q1, q2
vmulht.s16 q0, q1, q2
vpst
vmulh.s16 q0, q1, q2
it eq
vrmulheq.s16 q0, q1, q2
vrmulheq.s16 q0, q1, q2
vpst
vrmulheq.s16 q0, q1, q2
vrmulht.s16 q0, q1, q2
vpst
vrmulh.s16 q0, q1, q2
