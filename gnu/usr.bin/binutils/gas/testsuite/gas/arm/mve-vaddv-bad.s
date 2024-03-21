.syntax unified
.thumb

.irp cond, eq, ne, gt, ge, lt, le
.irp mnem, vaddv.s32, vaddva.u32

it \cond
\mnem r0, q0

.endr
.endr

vaddv.i32 r0, q0
vaddv.f32 r0, q0
vaddv.s64 r0, q0
vaddv.u64 r0, q0
vaddva.i32 r0, q0
vaddva.f32 r0, q0
vaddva.s64 r0, q0
vaddva.u64 r0, q0
vaddv.s32 r1, q0
it eq
vaddveq.s32 r0, q0
vaddveq.s32 r0, q0
vpst
vaddveq.s32 r0, q0
vaddvt.s32 r0, q0
vpst
vaddv.s32 r0, q0
it eq
vaddvaeq.s32 r0, q0
vaddvaeq.s32 r0, q0
vpst
vaddvaeq.s32 r0, q0
vaddvat.s32 r0, q0
vpst
vaddva.s32 r0, q0
