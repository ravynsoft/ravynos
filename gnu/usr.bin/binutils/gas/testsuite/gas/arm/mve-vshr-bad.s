.syntax unified
.thumb

vshr.s64 q0, q1, #1
vshr.i32 q0, q1, #1
vrshr.u64 q0, q1, #1
vrshr.i32 q0, q1, #1
vshr.s8 q0, q1, #9
vshr.u8 q0, q1, #9
vshr.s16 q0, q1, #17
vshr.u16 q0, q1, #17
vshr.s32 q0, q1, #33
vshr.u32 q0, q1, #33

.irp op, vshr, vrshr
.irp cond, eq, ne, gt, ge, lt, le
it \cond
\op\().s32 q0, q1, #1
.endr
.endr

it eq
vshreq.s32 q0, q1, #1
vshreq.s32 q0, q1, #1
vpst
vshreq.s32 q0, q1, #1
vshrt.s32 q0, q1, #1
vpst
vshr.s32 q0, q1, #1
it eq
vrshreq.s32 q0, q1, #1
vrshreq.s32 q0, q1, #1
vpst
vrshreq.s32 q0, q1, #1
vrshrt.s32 q0, q1, #1
vpst
vrshr.s32 q0, q1, #1
