.syntax unified
.thumb
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, q0, q1, q2, q4, q7
.irp data, u8, s8, i8, u16, f16, s32, f32
vorn.\data \op1, \op2, \op3
.endr
vorn \op1, \op2, \op3
.endr
.endr
.endr
vorn.i32 q0, #4294967295
vorn.i32 q7, #4294967295
vorn.i32 q0, #4294967040 @ ~0x000000FF
vorn.i32 q0, #4294902015 @ ~0x0000FF00
vorn.i32 q0, #4278255615 @ ~0xFF000000
vorn.i32 q0, #16777215   @ ~0x00FF0000
vorn.i16 q0, #4294967295
vorn.i16 q0, #255      @ 0x00FF
vorn.i16 q0, #65280    @ 0xFF00
vpstete
vornt.f16 q0, q1, q2
vorne q0, q1, q2
vornt.i32 q0, #4294967295
vorne.i16 q0, #4294967295
