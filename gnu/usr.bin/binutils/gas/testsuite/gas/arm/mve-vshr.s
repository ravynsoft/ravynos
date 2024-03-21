.syntax unified
.thumb

.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp data, s8, u8
.irp op3, #1, #2, #6, #7, #8
vshr.\data \op1, \op2, \op3
vrshr.\data \op1, \op2, \op3
.endr
.endr
.irp data, s16, u16
.irp op3, #1, #2, #6, #7, #8, #10, #14, #15, #16
vshr.\data \op1, \op2, \op3
vrshr.\data \op1, \op2, \op3
.endr
.endr
.irp data, s32, u32
.irp op3, #1, #2, #6, #7, #8, #10, #14, #15, #16, #24, #28, #30, #31, #32
vshr.\data \op1, \op2, \op3
vrshr.\data \op1, \op2, \op3
.endr
.endr
.endr
.endr

vpstete
vshrt.s8 q0, q1, #2
vshre.u16 q5, q0, #12
vrshrt.s32 q7, q7, #32
vrshre.u8 q7, q7, #8
