.syntax unified
.thumb
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, #1, #4, #6, #7, #8
vsri.8 \op1, \op2, \op3
.endr
.irp op3, #1, #4, #6, #7, #8, #12, #14, #15, #16
vsri.16 \op1, \op2, \op3
.endr
.irp op3, #1, #4, #6, #7, #8, #12, #14, #15, #16, #24, #28, #30, #31, #32
vsri.32 \op1, \op2, \op3
.endr
.endr
.endr
vpste
vsrit.8 q0, q1, #1
vsrie.32 q7, q7, #32
