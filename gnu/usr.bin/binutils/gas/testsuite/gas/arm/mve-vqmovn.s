.syntax unified
.thumb
.irp data, s16, u16, s32, u32
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
vqmovnt.\data \op1, \op2
vqmovnb.\data \op1, \op2
.endr
.endr
.endr
.irp data, s16, s32
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
vqmovunt.\data \op1, \op2
vqmovunb.\data \op1, \op2
.endr
.endr
.endr

vpstete
vqmovntt.s16 q0, q1
vqmovnte.u32 q7, q7
vqmovnbt.u16 q7, q7
vqmovnbe.s32 q0, q1
vqmovunt.s16 q0, q1
vqmovunt.s16 q7, q7
vqmovunt.s32 q0, q1
vqmovunt.s32 q7, q7
vqmovunb.s16 q0, q1
vqmovunb.s16 q7, q7
vqmovunb.s32 q0, q1
vqmovunb.s32 q7, q7
vpstete
vqmovuntt.s16 q0, q1
vqmovunte.s32 q7, q7
vqmovunbt.s16 q7, q7
vqmovunbe.s32 q0, q1
