.syntax unified
.thumb
.irp data, u8, s8, i8, u16, s16, i16, f16, u32, s32, i32, f32
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, q0, q1, q2, q4, q7
vand.\data \op1, \op2, \op3
.endr
.endr
.endr
.endr

.irp op1, q0, q1, q2, q4, q7
vand.i32 \op1, #4294967295
vand.i32 \op1, #4294967295
vand.i32 \op1, #4294967040 @ ~0x000000FF
vand.i32 \op1, #4294902015 @ ~0x0000FF00
vand.i32 \op1, #4278255615 @ ~0xFF000000
vand.i32 \op1, #16777215   @ ~0x00FF0000
vand.i16 \op1, #4294967295
vand.i16 \op1, #255      @ 0x00FF
vand.i16 \op1, #65280    @ 0xFF00
.endr
vpstete
vandt.f16 q0, q1, q2
vande q0, q1, q2
vandt.i32 q0, #4294967295
vande.i16 q0, #65280    @ 0xFF00
