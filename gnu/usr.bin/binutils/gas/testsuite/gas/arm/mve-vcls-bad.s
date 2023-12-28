.syntax unified
.thumb
vcls.f32 q0, q1
vcls.u32 q0, q1
vcls.32 q0, q1
vcls.i32 q0, q1
vcls.s64 q0, q1

.irp cond, eq, ne, gt, ge, lt, le

it \cond
vcls.s32 q0, q1

.endr

it eq
vclseq.s16 q0, q1
vclseq.s16 q0, q1
vpst
vclseq.s16 q0, q1
vclst.s16 q0, q1
vpst
vcls.s16 q0, q1

