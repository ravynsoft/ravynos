.syntax unified
.text
.thumb

.irp bt, b, t
vmull\bt\().s64 q0, q1, q2
vmull\bt\().f16 q0, q1, q2
vmull\bt\().f32 q0, q1, q2
vmull\bt\().s32 q1, q1, q2
vmull\bt\().s32 q2, q1, q2
.irp cond, eq, ne, gt, ge, lt, le
it \cond
vmull\bt\().s32 q0, q1, q2
.endr
.endr

it eq
vmullbeq.s32 q0, q1, q2
vmullbeq.s32 q0, q1, q2
vpst
vmullbeq.s32 q0, q1, q2
vpst
vmullb.s32 q0, q1, q2
vmullbt.s32 q0, q1, q2
it eq
vmullteq.s32 q0, q1, q2
vmullteq.s32 q0, q1, q2
vpst
vmullteq.s32 q0, q1, q2
vpst
vmullt.s32 q0, q1, q2
vmulltt.s32 q0, q1, q2
