.syntax unified
.thumb
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, q0, q1, q2, q4, q7
vsbc.i32 \op1, \op2, \op3
vsbci.i32 \op1, \op2, \op3
.endr
.endr
.endr

vpstete
vsbct.i32 q0, q1, q2
vsbce.i32 q7, q7, q7
vsbcit.i32 q6, q0, q3
vsbcie.i32 q5, q4, q0
