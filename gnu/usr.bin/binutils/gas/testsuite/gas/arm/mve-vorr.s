.syntax unified
.thumb
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, q0, q1, q2, q4, q7
.irp data, u8, s8, i8, u16, f16, s32, f32
vorr.\data \op1, \op2, \op3
.endr
vorr \op1, \op2, \op3
.endr
.endr
.endr
vorr.i32 q0, #0
vorr.i32 q0, #255        @ 0x000000FF 000000FF
vorr.i32 q0, #65280      @ 0x0000FF00 0000FF00
vorr.i32 q0, #4278190080 @ 0xFF000000 FF000000
vorr.i32 q0, #16711680   @ 0x00FF0000 00FF0000
vorr.i16 q0, #0
vorr.i16 q0, #255      @ 0x00FF 00FF 00FF 00FF
vorr.i16 q0, #65280    @ 0xFF00 FF00 FF00 FF00
vpstete
vorrt.f16 q0, q1, q2
vorre q0, q1, q2
vorrt.i32 q0, #0
vorre.i16 q0, #65280    @ 0xFF00 FF00 FF00 FF00
