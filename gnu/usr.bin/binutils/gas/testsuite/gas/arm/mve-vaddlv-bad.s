.syntax unified
.thumb

.irp cond, eq, ne, gt, ge, lt, le
.irp mnem, vaddlv.s32, vaddlva.u32

it \cond
\mnem r0, r1, q0

.endr
.endr

vaddlv.i32 r0, r1, q0
vaddlv.f32 r0, r1, q0
vaddlv.s8 r0, r1, q0
vaddlv.s16 r0, r1, q0
vaddlv.s64 r0, r1, q0
vaddlv.u8 r0, r1, q0
vaddlv.u16 r0, r1, q0
vaddlv.u64 r0, r1, q0
vaddlva.i32 r0, r1, q0
vaddlva.f32 r0, r1, q0
vaddlva.s8 r0, r1, q0
vaddlva.s16 r0, r1, q0
vaddlva.s64 r0, r1, q0
vaddlva.u8 r0, r1, q0
vaddlva.u16 r0, r1, q0
vaddlva.u64 r0, r1, q0
vaddlv.s32 r1, r3, q0
vaddlva.s32 r0, r2, q0
vaddlv.s32 r0, sp, q0
it eq
vaddlveq.s32 r0, r1, q0
vaddlveq.s32 r0, r1, q0
vpst
vaddlveq.s32 r0, r1, q0
vaddlvt.s32 r0, r1, q0
vpst
vaddlv.s32 r0, r1, q0
it eq
vaddlvaeq.s32 r0, r1, q0
vaddlvaeq.s32 r0, r1, q0
vpst
vaddlvaeq.s32 r0, r1, q0
vaddlvat.s32 r0, r1, q0
vpst
vaddlva.s32 r0, r1, q0
