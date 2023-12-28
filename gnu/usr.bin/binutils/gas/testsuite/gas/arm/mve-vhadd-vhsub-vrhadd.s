.syntax unified
.thumb
.irp data, u8, s8, u16, s16, u32, s32
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, q0, q1, q2, q4, q7
vrhadd.\data \op1, \op2, \op3
vhadd.\data \op1, \op2, \op3
vhsub.\data \op1, \op2, \op3
.endr
.irp op3, r0, r1, r2, r4, r7, r8, r10, r14
vhadd.\data \op1, \op2, \op3
vhsub.\data \op1, \op2, \op3
.endr
.endr
.endr
.endr

vpstete
vhaddt.s8 q0, q1, r2
vhadde.u16 q0, q1, q2
vhsubt.s32 q0, q1, r2
vhsube.u8 q0, q1, q2
vpste
vrhaddt.s16 q0, q1, q2
vrhadde.u32 q0, q1, q2
