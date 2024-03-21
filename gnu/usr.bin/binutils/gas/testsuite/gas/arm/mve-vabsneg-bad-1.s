.macro cond op
.irp cond, eq, ne, gt, ge, lt, le
it \cond
\op\().s32 q0, q1
.endr
.endm



.syntax unified
.text
.thumb
vabs.p8 q0, q1
vabs.p16 q0, q1
vabs.u8 q0, q1
vabs.u16 q0, q1
vabs.u32 q0, q1
vabs.f16 q0, q1
vabs.f32 q0, q1
vabs.s64 q0, q1
cond vabs
vneg.p8 q0, q1
vneg.p16 q0, q1
vneg.u8 q0, q1
vneg.u16 q0, q1
vneg.u32 q0, q1
vneg.f16 q0, q1
vneg.f32 q0, q1
vneg.s64 q0, q1
cond vneg
it eq
vnegeq.s32 q0, q1
vnegeq.s32 q0, q1
vpst
vneg.s32 q0, q1
vnegt.s32 q0, q1

