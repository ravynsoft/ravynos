.syntax unified
.thumb
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, #0, #1, #2, #4, #7
vsli.8 \op1, \op2, \op3
.endr
.irp op3, #0, #1, #2, #4, #7, #8, #10, #12, #14, #15
vsli.16 \op1, \op2, \op3
.endr
.irp op3, #0, #1, #2, #4, #7, #8, #10, #12, #14, #15, #16, #20, #24, #28, #31
vsli.32 \op1, \op2, \op3
.endr
.endr
.endr

vpste
vslit.8 q0, q1, #0
vslie.32 q7, q7, #31
