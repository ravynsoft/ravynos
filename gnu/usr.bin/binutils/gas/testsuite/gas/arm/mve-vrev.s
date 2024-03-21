.syntax unified
.thumb

.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
vrev16.8 \op1, \op2
.irp data, 8, 16
vrev32.\data \op1, \op2
.endr
.endr
.endr

.irp data, 8, 16, 32
.irp op2, q1, q2, q4, q7
vrev64.\data q0, \op2
.endr
.endr
.irp data, 8, 16, 32
.irp op2, q0, q2, q4, q7
vrev64.\data q1, \op2
.endr
.endr
.irp data, 8, 16, 32
.irp op2, q0, q1, q4, q7
vrev64.\data q2, \op2
.endr
.endr
.irp data, 8, 16, 32
.irp op2, q0, q1, q2, q7
vrev64.\data q4, \op2
.endr
.endr
.irp data, 8, 16, 32
.irp op2, q0, q1, q2, q4
vrev64.\data q7, \op2
.endr
.endr

vpstete
vrev16t.8 q0, q1
vrev16e.8 q7, q7
vrev32t.8 q7, q7
vrev32e.16 q0, q1
vpste
vrev64t.32 q0, q1
vrev64e.32 q7, q6
