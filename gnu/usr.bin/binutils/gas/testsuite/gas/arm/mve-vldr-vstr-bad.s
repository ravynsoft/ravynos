.syntax unified
.thumb

.irp op1, 8, 16, 32
.irp op2, q0, q1, q2, q4, q7
.irp op3, r0, r1, r2, r4, r7, r8, r10, r12, r14
vldrb.s\op1 \op2, \op3
vldrb.u\op1 \op2, \op3
vstrb.\op1 \op2, \op3
.endr
.endr
.endr

.irp op1, 16, 32
.irp op2, q0, q1, q2, q4, q7
.irp op3, r0, r1, r2, r4, r7, r8, r10, r12, r14
vldrh.s\op1 \op2, \op3
vldrh.u\op1 \op2, \op3
vstrh.\op1 \op2, \op3
.endr
.endr
.endr

.irp op2, q0, q1, q2, q4, q7
.irp op3, r0, r1, r2, r4, r7, r8, r10, r12, r14
vldrw.s32 \op2, \op3
vldrw.u32 \op2, \op3
vstrw.32 \op2, \op3
.endr
.endr
