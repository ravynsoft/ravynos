.syntax unified
.thumb
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, q0, q1, q2, q4, q7
.irp data, u8, s8, i8, u16, f16, s32, f32
vbic.\data \op1, \op2, \op3
.endr
vbic \op1, \op2, \op3
.endr
.endr
.endr

vbic.i32 q0, #0
vbic.i32 q0, #255        @ 0x000000FF 000000FF
vbic.i32 q0, #65280      @ 0x0000FF00 0000FF00
vbic.i32 q0, #4278190080 @ 0xFF000000 FF000000
vbic.i32 q0, #16711680   @ 0x00FF0000 00FF0000
vbic.i16 q0, #0
vbic.i16 q0, #255      @ 0x00FF 00FF 00FF 00FF
vbic.i16 q0, #65280    @ 0xFF00 FF00 FF00 FF00
vpstete
vbict.f16 q0, q1, q2
vbice q0, q1, q2
vbict.i32 q0, #0
vbice.i16 q0, #65280    @ 0xFF00 FF00 FF00 FF00
