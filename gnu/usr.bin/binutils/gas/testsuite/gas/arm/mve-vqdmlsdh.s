.syntax unified
.thumb
.irp data, s8, s16
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, q0, q1, q2, q4, q7
vqdmlsdh.\data \op1, \op2, \op3
vqdmlsdhx.\data \op1, \op2, \op3
vqrdmlsdh.\data \op1, \op2, \op3
vqrdmlsdhx.\data \op1, \op2, \op3
.endr
.endr
.endr
.endr

.irp op2, q1, q2, q4, q7
.irp op3, q1, q2, q4, q7
vqdmlsdh.s32 q0, \op2, \op3
vqdmlsdhx.s32 q0, \op2, \op3
vqrdmlsdh.s32 q0, \op2, \op3
vqrdmlsdhx.s32 q0, \op2, \op3
.endr
.endr
.irp op2, q0, q2, q4, q7
.irp op3, q0, q2, q4, q7
vqdmlsdh.s32 q1, \op2, \op3
vqdmlsdhx.s32 q1, \op2, \op3
vqrdmlsdh.s32 q1, \op2, \op3
vqrdmlsdhx.s32 q1, \op2, \op3
.endr
.endr
.irp op2, q0, q1, q4, q7
.irp op3, q0, q1, q4, q7
vqdmlsdh.s32 q2, \op2, \op3
vqdmlsdhx.s32 q2, \op2, \op3
vqrdmlsdh.s32 q2, \op2, \op3
vqrdmlsdhx.s32 q2, \op2, \op3
.endr
.endr
.irp op2, q0, q1, q4, q7
.irp op3, q0, q1, q4, q7
vqdmlsdh.s32 q2, \op2, \op3
vqdmlsdhx.s32 q2, \op2, \op3
vqrdmlsdh.s32 q2, \op2, \op3
vqrdmlsdhx.s32 q2, \op2, \op3
.endr
.endr
.irp op2, q0, q1, q2, q7
.irp op3, q0, q1, q2, q7
vqdmlsdh.s32 q4, \op2, \op3
vqdmlsdhx.s32 q4, \op2, \op3
vqrdmlsdh.s32 q4, \op2, \op3
vqrdmlsdhx.s32 q4, \op2, \op3
.endr
.endr
.irp op2, q0, q1, q2, q4
.irp op3, q0, q1, q2, q4
vqdmlsdh.s32 q7, \op2, \op3
vqdmlsdhx.s32 q7, \op2, \op3
vqrdmlsdh.s32 q7, \op2, \op3
vqrdmlsdhx.s32 q7, \op2, \op3
.endr
.endr
vpstete
vqdmlsdht.s8 q0, q1, q2
vqdmlsdhe.s8 q0, q1, q2
vqdmlsdhxt.s16 q0, q1, q2
vqdmlsdhxe.s16 q0, q1, q2
vpstete
vqrdmlsdht.s32 q0, q1, q2
vqrdmlsdhe.s32 q0, q1, q2
vqrdmlsdhxt.s16 q0, q1, q2
vqrdmlsdhxe.s16 q0, q1, q2
vqdmlsdh.s32 q0, q0, q0
vqrdmlsdh.s32 q0, q0, q0
vqdmlsdh.s32 q1, q1, q2
vqrdmlsdh.s32 q2, q2, q3
vqdmlsdh.s32 q3, q4, q3
vqrdmlsdh.s32 q4, q5, q4
