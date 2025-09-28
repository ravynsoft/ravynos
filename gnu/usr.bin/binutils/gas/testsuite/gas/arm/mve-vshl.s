.syntax unified
.thumb
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, #0, #1, #2, #4, #7
vshl.i8 \op1, \op2, \op3
.endr
.irp op3, #0, #1, #2, #4, #7, #8, #10, #12, #15
vshl.i16 \op1, \op2, \op3
.endr
.irp op3, #0, #1, #2, #4, #7, #8, #10, #12, #15, #16, #18, #20, #24, #30
vshl.i32 \op1, \op2, \op3
.endr
.endr
.endr

.irp data, u8, s8, u16, s16, u32, s32
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, q0, q1, q2, q4, q7
vshl.\data \op1, \op2, \op3
.endr
.endr
.irp op2, r0, r1, r2, r4, r7, r8, r10, r12, r14
vshl.\data \op1, \op2
.endr
.endr
.endr

vpstete
vshlt.i8 q0, q1, #0
vshle.i16 q7, q0, #5
vshlt.s32 q0, r4
vshle.u8 q5, r12
vpste
vshlt.s16 q0, q4, q6
vshle.u32 q2, q5, q7
