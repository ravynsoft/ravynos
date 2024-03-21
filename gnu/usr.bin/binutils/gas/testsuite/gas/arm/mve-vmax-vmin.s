.syntax unified
.thumb
.irp data, s8, u8, s16, u16, s32, u32
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, q0, q1, q2, q4, q7
vmax.\data \op1, \op2, \op3
vmin.\data \op1, \op2, \op3
.endr
.endr
.endr
.endr
vpstete
vmaxt.s8 q0, q1, q2
vmaxe.u16 q7, q7, q7
vmint.s32 q0, q1, q2
vmine.u8 q7, q7, q7
