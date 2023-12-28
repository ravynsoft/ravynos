.syntax unified
.text
.thumb
vabav.s64 r0, q0, q1
vabav.f16 r0, q0, q1
vabav.f32 r0, q0, q1
vabav.p8 r0, q0, q1
vabav.p16 r0, q0, q1
vabav.s32 r13, q0, q1
vabav.s32 r15, q0, q1

.irp cond, eq, ne, gt, ge, lt, le
it \cond
vabav.s32 r0, q0, q1
.endr

vpst
vabaveq.s32 r0, q0, q1
vabaveq.s32 r0, q0, q1
it eq
vabavt.s32 r0, q0, q1
vabavt.s32 r0, q0, q1
