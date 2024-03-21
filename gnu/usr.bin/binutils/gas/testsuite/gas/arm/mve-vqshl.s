.syntax unified
.thumb
.irp data, u8, s8, u16, s16, u32, s32
.irp op1, q0, q1, q2, q4, q7
.irp op2, r0, r1, r2, r4, r7, r8, r10, r12, r14
vqshl.\data \op1, \op2
.endr
.irp op2, q0, q1, q2, q4, q7
.irp op3, q0, q1, q2, q4, q7
vqshl.\data \op1, \op2, \op3
.endr
.endr
.endr
.endr

.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, #0, #1, #2, #4, #7
vqshl.u8 \op1, \op2, \op3
vqshl.s8 \op1, \op2, \op3
vqshlu.s8 \op1, \op2, \op3
.endr
.irp op3, #0, #1, #2, #4, #7, #8, #10, #12, #14, #15
vqshl.u16 \op1, \op2, \op3
vqshl.s16 \op1, \op2, \op3
vqshlu.s16 \op1, \op2, \op3
.endr
.irp op3, #0, #1, #2, #4, #7, #8, #10, #12, #14, #15, #16, #20, #24, #30
vqshl.u32 \op1, \op2, \op3
vqshl.s32 \op1, \op2, \op3
vqshlu.s32 \op1, \op2, \op3
.endr
.endr
.endr

vpstete
vqshlt.s8 q3, #7
vqshle.s16 q5, #12
vqshlt.u32 q6, r8
vqshle.s8 q1, r10
vpstete
vqshlt.u16 q0, q1, q2
vqshle.s32 q7, q6, q5
vqshlut.s16 q0, q1, #2
vqshlue.s32 q5, q3, #15
