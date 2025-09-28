.syntax unified
.thumb

vmaxv.u64 r0, q1
vmaxv.f16 r0, q1
vminv.s64 r0, q1
vminv.f32 r0, q1
vmaxav.u16 r0, q1
vmaxav.f32 r0, q1
vminav.u32 r0, q1
vminav.f16 r0, q1
vmaxv.s32 sp, q1
vmaxav.s32 pc, q1
vminv.s32 pc, q1
vminav.s32 sp, q1

.irp op, vmaxv, vmaxav, vminv, vminav
.irp cond, eq, ne, gt, ge, lt, le

it \cond
\op\().s16 r0, q1

.endr
.endr

it eq
vmaxveq.s32 r0, q1
vmaxveq.s32 r0, q1
vpst
vmaxveq.s32 r0, q1
vmaxvt.s32 r0, q1
vpst
vmaxv.s32 r0, q1
it eq
vmaxaveq.s32 r0, q1
vmaxaveq.s32 r0, q1
vpst
vmaxaveq.s32 r0, q1
vmaxavt.s32 r0, q1
vpst
vmaxav.s32 r0, q1
it eq
vminveq.s32 r0, q1
vminveq.s32 r0, q1
vpst
vminveq.s32 r0, q1
vminvt.s32 r0, q1
vpst
vminv.s32 r0, q1
it eq
vminaveq.s32 r0, q1
vminaveq.s32 r0, q1
vpst
vminaveq.s32 r0, q1
vminavt.s32 r0, q1
vpst
vminav.s32 r0, q1
