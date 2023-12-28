.syntax unified
.thumb

.irp round, a, n, p, m
.irp cond, eq, ne, gt, ge, lt, le
.irp size, .s16.f16, .u16.f16, .s32.f32, .u32.f32

it \cond
vcvt\round\size q0, q1

.endr
.endr
.endr

vcvta.s64.f64 q0, q1
vcvta.u64.f64 q0, q1
vcvta.f64.s64 q0, q1
vcvta.f64.u64 q0, q1
vcvtn.s64.f64 q0, q1
vcvtn.u64.f64 q0, q1
vcvtn.f64.s64 q0, q1
vcvtn.f64.u64 q0, q1
vcvtp.s64.f64 q0, q1
vcvtp.u64.f64 q0, q1
vcvtp.f64.s64 q0, q1
vcvtp.f64.u64 q0, q1
vcvtm.s64.f64 q0, q1
vcvtm.u64.f64 q0, q1
vcvtm.f64.s64 q0, q1
vcvtm.f64.u64 q0, q1
it eq
vcvtaeq.s32.f32 q0, q1
vcvtaeq.s32.f32 q0, q1
vpst
vcvtaeq.s32.f32 q0, q1
vcvtat.s32.f32 q0, q1
vpst
vcvta.s32.f32 q0, q1
it eq
vcvtneq.s32.f32 q0, q1
vcvtneq.s32.f32 q0, q1
vpst
vcvtneq.s32.f32 q0, q1
vcvtnt.s32.f32 q0, q1
vpst
vcvtn.s32.f32 q0, q1
it eq
vcvtpeq.s32.f32 q0, q1
vcvtpeq.s32.f32 q0, q1
vpst
vcvtpeq.s32.f32 q0, q1
vcvtpt.s32.f32 q0, q1
vpst
vcvtp.s32.f32 q0, q1
it eq
vcvtmeq.s32.f32 q0, q1
vcvtmeq.s32.f32 q0, q1
vpst
vcvtmeq.s32.f32 q0, q1
vcvtmt.s32.f32 q0, q1
vpst
vcvtm.s32.f32 q0, q1
