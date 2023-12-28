.macro cond mnem
.irp cond, eq, ne, gt, ge, lt, le
it \cond
\mnem\().32 q0, [r0, q1]
.endr
.endm



.syntax unified
.thumb
vstrb.s8 q0, [r0, q1]
vstrb.u8 q0, [r0, q1]
vstrb.s16 q0, [r0, q1]
vstrb.u16 q0, [r0, q1]
vstrb.f16 q0, [r0, q1]
vstrb.u32 q0, [r0, q1]
vstrb.s32 q0, [r0, q1]
vstrb.f32 q0, [r0, q1]
vstrb.64 q0, [r0, q1]
vstrb.16 q0, [pc, q1]
cond vstrb


vstrh.8 q0, [r0, q1]
vstrh.s8 q0, [r0, q1]
vstrh.u8 q0, [r0, q1]
vstrh.s16 q0, [r0, q1]
vstrh.u16 q0, [r0, q1]
vstrh.f16 q0, [r0, q1]
vstrh.u32 q0, [r0, q1]
vstrh.s32 q0, [r0, q1]
vstrh.f32 q0, [r0, q1]
vstrh.64 q0, [r0, q1]
vstrh.16 q0, [pc, q1]
cond vstrh


vstrh.16 q0, [r0, q1, #1]
vstrh.16 q0, [r0, q1, UXTW #2]
vstrw.8 q0, [r0, q1]
vstrw.u8 q0, [r0, q1]
vstrw.s8 q0, [r0, q1]
vstrw.16 q0, [r0, q1]
vstrw.f16 q0, [r0, q1]
vstrw.u16 q0, [r0, q1]
vstrw.s16 q0, [r0, q1]
vstrw.u32 q0, [r0, q1]
vstrw.s32 q0, [r0, q1]
vstrw.f32 q0, [r0, q1]
vstrw.64 q0, [r0, q1]
vstrw.32 q0, [pc, q1]
cond vstrw


vstrw.32 q0, [r0, q1, #2]
vstrw.32 q0, [r0, q1, UXTW #1]
vstrw.32 q0, [r0, q1, UXTW #3]
vstrd.8 q0, [r0, q1]
vstrd.u8 q0, [r0, q1]
vstrd.s8 q0, [r0, q1]
vstrd.16 q0, [r0, q1]
vstrd.u16 q0, [r0, q1]
vstrd.s16 q0, [r0, q1]
vstrd.f16 q0, [r0, q1]
vstrd.32 q0, [r0, q1]
vstrd.u32 q0, [r0, q1]
vstrd.s32 q0, [r0, q1]
vstrd.f32 q0, [r0, q1]
vstrd.f64 q0, [r0, q1]
vstrd.u64 q0, [r0, q1]
vstrd.s64 q0, [r0, q1]

.macro cond64
.irp cond, eq, ne, gt, ge, lt, le
it \cond
vstrd\().64 q0, [r0, q1]
.endr
.endm



cond64
vstrd.64 q0, [r0, q1, #3]
vstrd.64 q0, [r0, q1, UXTW #1]
vstrd.64 q0, [r0, q1, UXTW #2]
vstrd.64 q0, [r0, q1, UXTW #4]

it eq
vstrbeq.32 q0, [r0, q1]
vstrbeq.32 q0, [r0, q1]
vpst
vstrbeq.32 q0, [r0, q1]
vpst
vstrb.32 q0, [r0, q1]
vstrbt.32 q0, [r0, q1]
vstrbe.32 q0, [r0, q1]
it eq
vstrheq.32 q0, [r0, q1]
vstrheq.32 q0, [r0, q1]
vpst
vstrheq.32 q0, [r0, q1]
vpst
vstrh.32 q0, [r0, q1]
vstrht.32 q0, [r0, q1]
vstrhe.32 q0, [r0, q1]
it eq
vstrweq.32 q0, [r0, q1]
vstrweq.32 q0, [r0, q1]
vpst
vstrweq.32 q0, [r0, q1]
vpst
vstrw.32 q0, [r0, q1]
vstrwt.32 q0, [r0, q1]
vstrwe.32 q0, [r0, q1]
it eq
vstrdeq.64 q0, [r0, q1]
vstrdeq.64 q0, [r0, q1]
vpst
vstrdeq.64 q0, [r0, q1]
vpst
vstrd.64 q0, [r0, q1]
vstrdt.64 q0, [r0, q1]
vstrde.64 q0, [r0, q1]
