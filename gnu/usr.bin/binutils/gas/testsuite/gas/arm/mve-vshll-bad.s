.syntax unified
.thumb

vshllt.s32 q0, q1, #1
vshllt.i8 q0, q1, #1
vshllt.u8 q0, q1, #0
vshllt.u8 q0, q1, #9
vshllt.s16 q0, q1, #0
vshllt.s16 q0, q1, #17
vshllb.s32 q0, q1, #1
vshllb.i8 q0, q1, #1
vshllb.u8 q0, q1, #0
vshllb.u8 q0, q1, #9
vshllb.s16 q0, q1, #0
vshllb.s16 q0, q1, #17

.irp op, vshllt, vshllb
.irp cond, eq, ne, gt, ge, lt, le
it \cond
\op\().s16 q0, q1, #4
.endr
.endr

it eq
vshllteq.s8 q0, q1, #1
vshllteq.s8 q0, q1, #1
vpst
vshllteq.s8 q0, q1, #1
vshlltt.s8 q0, q1, #1
vpst
vshllt.s8 q0, q1, #1
it eq
vshllbeq.s8 q0, q1, #1
vshllbeq.s8 q0, q1, #1
vpst
vshllbeq.s8 q0, q1, #1
vshllbt.s8 q0, q1, #1
vpst
vshllb.s8 q0, q1, #1
