.syntax unified
.thumb

.irp data, s8, s16
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, q0, q1, q2, q4, q7
vqdmladh.\data \op1, \op2, \op3
vqdmladhx.\data \op1, \op2, \op3
vqrdmladh.\data \op1, \op2, \op3
vqrdmladhx.\data \op1, \op2, \op3
.endr
.endr
.endr
.endr

.irp op2, q1, q2, q4, q7
.irp op3, q1, q2, q4, q7
vqdmladh.s32 q0, \op2, \op3
vqdmladhx.s32 q0, \op2, \op3
vqrdmladh.s32 q0, \op2, \op3
vqrdmladhx.s32 q0, \op2, \op3
.endr
.endr
.irp op2, q0, q2, q4, q7
.irp op3, q0, q2, q4, q7
vqdmladh.s32 q1, \op2, \op3
vqdmladhx.s32 q1, \op2, \op3
vqrdmladh.s32 q1, \op2, \op3
vqrdmladhx.s32 q1, \op2, \op3
.endr
.endr
.irp op2, q0, q1, q4, q7
.irp op3, q0, q1, q4, q7
vqdmladh.s32 q2, \op2, \op3
vqdmladhx.s32 q2, \op2, \op3
vqrdmladh.s32 q2, \op2, \op3
vqrdmladhx.s32 q2, \op2, \op3
.endr
.endr
.irp op2, q0, q1, q4, q7
.irp op3, q0, q1, q4, q7
vqdmladh.s32 q2, \op2, \op3
vqdmladhx.s32 q2, \op2, \op3
vqrdmladh.s32 q2, \op2, \op3
vqrdmladhx.s32 q2, \op2, \op3
.endr
.endr
.irp op2, q0, q1, q2, q7
.irp op3, q0, q1, q2, q7
vqdmladh.s32 q4, \op2, \op3
vqdmladhx.s32 q4, \op2, \op3
vqrdmladh.s32 q4, \op2, \op3
vqrdmladhx.s32 q4, \op2, \op3
.endr
.endr
.irp op2, q0, q1, q2, q4
.irp op3, q0, q1, q2, q4
vqdmladh.s32 q7, \op2, \op3
vqdmladhx.s32 q7, \op2, \op3
vqrdmladh.s32 q7, \op2, \op3
vqrdmladhx.s32 q7, \op2, \op3
.endr
.endr

vpstete
vqdmladht.s8 q0, q1, q2
vqdmladhe.s8 q0, q1, q2
vqdmladhxt.s16 q0, q1, q2
vqdmladhxe.s16 q0, q1, q2
vpstete
vqrdmladht.s32 q0, q1, q2
vqrdmladhe.s32 q0, q1, q2
vqrdmladhxt.s16 q0, q1, q2
vqrdmladhxe.s16 q0, q1, q2
vqdmladh.s32 q0, q0, q0
vqrdmladh.s32 q0, q0, q0
vqdmladh.s32 q0, q0, q1
vqrdmladh.s32 q1, q1, q2
vqdmladh.s32 q2, q3, q2
vqrdmladh.s32 q3, q4, q3
