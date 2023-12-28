.syntax unified
.thumb
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, #1, #2, #6, #7, #8
vshrnt.i16 \op1, \op2, \op3
vshrnb.i16 \op1, \op2, \op3
vrshrnt.i16 \op1, \op2, \op3
vrshrnb.i16 \op1, \op2, \op3
.endr
.irp op3, #1, #2, #6, #7, #8, #10, #14, #15, #16
vshrnt.i32 \op1, \op2, \op3
vshrnb.i32 \op1, \op2, \op3
vrshrnt.i32 \op1, \op2, \op3
vrshrnb.i32 \op1, \op2, \op3
.endr
.endr
.endr

vpstete
vshrntt.i16 q0, q1, #2
vshrnte.i32 q7, q7, #16
vshrnbt.i16 q7, q7, #8
vshrnbe.i32 q4, q5, #7
vpstete
vrshrntt.i16 q0, q1, #2
vrshrnte.i32 q7, q7, #16
vrshrnbt.i16 q7, q7, #8
vrshrnbe.i32 q4, q5, #7
