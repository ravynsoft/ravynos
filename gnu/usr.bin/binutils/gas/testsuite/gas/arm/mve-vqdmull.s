.syntax unified
.thumb

.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, r0, r1, r2, r4, r7, r8, r10, r12, r14
vqdmullt.s16 \op1, \op2, \op3
vqdmullb.s16 \op1, \op2, \op3
.endr
.endr
.endr

.irp op2, q1, q2, q4, q7
.irp op3, r0, r2, r4, r7, r8, r10, r12, r14
vqdmullt.s32 q0, \op2, \op3
vqdmullb.s32 q0, \op2, \op3
.endr
.endr

.irp op2, q0, q2, q4, q7
.irp op3, r0, r2, r4, r7, r8, r10, r12, r14
vqdmullt.s32 q1, \op2, \op3
vqdmullb.s32 q1, \op2, \op3
.endr
.endr

.irp op2, q0, q1, q4, q7
.irp op3, r0, r2, r4, r7, r8, r10, r12, r14
vqdmullt.s32 q2, \op2, \op3
vqdmullb.s32 q2, \op2, \op3
.endr
.endr

.irp op2, q0, q1, q2, q7
.irp op3, r0, r2, r4, r7, r8, r10, r12, r14
vqdmullt.s32 q4, \op2, \op3
vqdmullb.s32 q4, \op2, \op3
.endr
.endr

.irp op2, q0, q1, q2, q4
.irp op3, r0, r2, r4, r7, r8, r10, r12, r14
vqdmullt.s32 q7, \op2, \op3
vqdmullb.s32 q7, \op2, \op3
.endr
.endr
vpstete
vqdmulltt.s16 q0, q1, q2
vqdmullte.s32 q0, q1, q2
vqdmullbt.s16 q0, q1, q2
vqdmullbe.s32 q0, q1, q2
vpstete
vqdmulltt.s16 q7, q7, lr
vqdmullte.s32 q7, q6, r0
vqdmullbt.s16 q0, q1, r2
vqdmullbe.s32 q5, q7, r14
