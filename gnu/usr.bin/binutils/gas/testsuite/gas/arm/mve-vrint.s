.syntax unified
.thumb
.irp op, n, x, a, z, m, p
.irp data, f16, f32
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
vrint\op\().\data \op1, \op2
.endr
.endr
.endr
.endr
vpstete
vrintnt.f16 q0, q1
vrintne.f32 q7, q7
vrintxt.f32 q0, q1
vrintxe.f16 q7, q7
vpstete
vrintat.f16 q0, q1
vrintae.f32 q7, q7
vrintzt.f32 q0, q1
vrintze.f16 q7, q7
vpstete
vrintmt.f16 q0, q1
vrintme.f32 q7, q7
vrintpt.f32 q0, q1
vrintpe.f16 q7, q7
