.syntax unified
.thumb
.irp data, u8, u16, u32
.irp op1, q0, q1, q2, q4, q7
.irp op2, r0, r2, r4, r6, r8, r10, r12
.irp imm, #1, #2, #4, #8
vddup.\data \op1, \op2, \imm
vidup.\data \op1, \op2, \imm
.endr
.irp op3, r1, r3, r5, r7, r9, r11
.irp imm, #1, #2, #4, #8
vdwdup.\data \op1, \op2, \op3, \imm
viwdup.\data \op1, \op2, \op3, \imm
.endr
.endr
.endr
.endr
.endr
vpstet
vdwdupt.u8 q0, r0, r1, #1
vdwdupe.u16 q0, r0, r1, #4
vdwdupt.u32 q2, r4, r7, #1
vpstet
vddupt.u8 q0, r0, #2
vddupe.u16 q7, r0, #1
vddupt.u32 q4, r8, #1
vpstet
viwdupt.u8 q0, r0, r1, #1
viwdupe.u16 q0, r0, r1, #4
viwdupt.u32 q2, r4, r7, #1
vpstet
vidupt.u8 q0, r0, #2
vidupe.u16 q7, r0, #1
vidupt.u32 q4, r8, #1
