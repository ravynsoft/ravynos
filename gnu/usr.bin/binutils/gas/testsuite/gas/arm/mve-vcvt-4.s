.syntax unified
.thumb

.irp round, a, n, p, m
.irp conv, .s16.f16, .u16.f16, .s32.f32, .u32.f32
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
vcvt\round\conv \op1, \op2
.endr
.endr
.endr
.endr

vpstttt
vcvtat.s16.f16 q0, q1
vcvtnt.u16.f16 q3, q7
vcvtmt.s32.f32 q1, q2
vcvtpt.u32.f32 q4, q5

vpstett
vcvtat.s16.f16 q0, q1
vcvtne.u16.f16 q3, q7
vcvtmt.s32.f32 q1, q2
vcvtpt.u32.f32 q4, q5

vpsttet
vcvtat.s16.f16 q0, q1
vcvtnt.u16.f16 q3, q7
vcvtme.s32.f32 q1, q2
vcvtpt.u32.f32 q4, q5

vpsttte
vcvtat.s16.f16 q0, q1
vcvtnt.u16.f16 q3, q7
vcvtmt.s32.f32 q1, q2
vcvtpe.u32.f32 q4, q5

vpstete
vcvtnt.u16.f16 q3, q7
vcvtae.s16.f16 q0, q1
vcvtmt.s32.f32 q1, q2
vcvtpe.u32.f32 q4, q5
