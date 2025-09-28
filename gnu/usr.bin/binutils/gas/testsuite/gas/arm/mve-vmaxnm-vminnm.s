.syntax unified
.thumb
.irp data, f16, f32
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
vmaxnma.\data \op1, \op2
vminnma.\data \op1, \op2
.irp op3, q0, q1, q2, q4, q7
vmaxnm.\data \op1, \op2, \op3
vminnm.\data \op1, \op2, \op3
.endr
.endr
.endr
.endr
vpstete
vmaxnmt.f16 q0, q1, q2
vmaxnme.f32 q7, q7, q7
vminnmt.f32 q0, q1, q2
vminnme.f16 q7, q7, q7
vpstete
vmaxnmat.f16 q0, q1
vmaxnmae.f32 q7, q7
vminnmat.f32 q0, q1
vminnmae.f16 q7, q7
