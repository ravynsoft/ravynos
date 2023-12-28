.syntax unified
.thumb
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
vmvn \op1, \op2
.endr
vmvn.i32 \op1, #0
vmvn.i16 \op1, #0
vmvn.i32 \op1, #0x000000FF   @ cmode 0000
vmvn.i32 \op1, #0x0000FF00   @ cmode 0010
vmvn.i32 \op1, #0x00FF0000   @ cmode 0100
vmvn.i32 \op1, #0xFF000000   @ cmode 0110
vmvn.i32 \op1, #0x0000ABFF   @ cmode 1100
vmvn.i16 \op1, #0x00FF       @ cmode 1000
vmvn.i16 \op1, #0xFF00       @ cmode 1010
vmvn.i16 \op1, #0x1ff	     @ becomes vmov with 0xfe00
vmvn.i8  \op1, #0xff         @ becomes vmov with 0x00
.endr
vpstete
vmvnt.i32 q0, #0x000000FF
vmvne.i16 q6, #0xFF00
vmvnt q0, q1
vmvne q7, q3
