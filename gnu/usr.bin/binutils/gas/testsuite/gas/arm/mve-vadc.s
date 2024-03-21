.syntax unified
.thumb

.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, q0, q1, q2, q4, q7
vadc.i32 \op1, \op2, \op3
vadci.i32 \op1, \op2, \op3
.endr
.endr
.endr

vpstete
vadct.i32 q0, q1, q2
vadce.i32 q7, q7, q7
vadcit.i32 q0, q1, q2
vadcie.i32 q7, q7, q7
