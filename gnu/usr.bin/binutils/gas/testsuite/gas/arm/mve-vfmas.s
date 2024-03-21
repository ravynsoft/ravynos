.syntax unified
.thumb
.irp data, f16, f32
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, r0, r1, r2, r4, r7, r8, r10, r14
vfmas.\data \op1, \op2, \op3
.endr
.endr
.endr
.endr
vpstete
vfmast.f16 q0, q1, r2
vfmase.f16 q7, q7, r14
vfmast.f32 q0, q1, r2
vfmase.f32 q6, q3, r8
