.syntax unified
.thumb
.irp data, f16, f32
.irp op1, r0, r1, r2, r4, r7, r8, r10, r12, r14
.irp op2, q0, q1, q2, q4, q7
vmaxnmv.\data \op1, \op2
vmaxnmav.\data \op1, \op2
vminnmv.\data \op1, \op2
vminnmav.\data \op1, \op2
.endr
.endr
.endr
vpstete
vmaxnmvt.f16 r0, q2
vmaxnmve.f32 r2, q4
vminnmvt.f32 r7, q6
vminnmve.f16 r12, q1
vpstete
vmaxnmavt.f16 r0, q2
vmaxnmave.f32 r2, q4
vminnmavt.f32 r7, q6
vminnmave.f16 r12, q1
