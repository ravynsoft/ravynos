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
vabs.s64 q0, q1
cond vabs
vneg.p8 q0, q1
vneg.p16 q0, q1
vneg.u8 q0, q1
vneg.u16 q0, q1
vneg.u32 q0, q1
vneg.s64 q0, q1
cond vneg
it eq
vnegeq.f32 q0, q1
vnegeq.f32 q0, q1
vpst
vneg.f32 q0, q1
vnegt.f32 q0, q1
