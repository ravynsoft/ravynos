.syntax unified
.thumb

.irp data, f16, f32
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, r0, r1, r2, r4, r7, r8, r10, r14
vfma.\data \op1, \op2, \op3
.endr
.irp op3, q0, q1, q2, q4, q7
vfma.\data \op1, \op2, \op3
vfms.\data \op1, \op2, \op3
.endr
.endr
.endr
.endr

vpstete
vfmat.f16 q0, q1, q2
vfmae.f32 q7, q7, q7
vfmat.f16 q0, q1, r2
vfmae.f32 q7, q7, r12
vpste
vfmst.f16 q0, q1, q2
vfmse.f32 q7, q7, q7
