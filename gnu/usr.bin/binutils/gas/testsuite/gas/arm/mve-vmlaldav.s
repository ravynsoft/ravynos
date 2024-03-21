.syntax unified
.thumb

.irp op1, r0, r2, r4, r8, r10, r12, r14
.irp op2, r1, r3, r5, r7, r9, r11
.irp op3, q0, q1, q2, q4, q7
.irp op4, q0, q1, q2, q4, q7
.irp data, .s16, .s32
vmlaldav\data \op1, \op2, \op3, \op4
vmlaldava\data \op1, \op2, \op3, \op4
vmlaldavx\data \op1, \op2, \op3, \op4
vmlaldavax\data \op1, \op2, \op3, \op4
.endr
.irp data, .u16, .u32
vmlaldav\data \op1, \op2, \op3, \op4
vmlaldava\data \op1, \op2, \op3, \op4
.endr
.endr
.endr
.endr
.endr

vpstete
vmlaldavt.u16 r0, r1, q2, q3
vmlaldave.s32 r0, r1, q2, q3
vmlaldavat.u32 r0, r1, q2, q3
vmlaldavae.s16 r0, r1, q2, q3
vpstete
vmlaldavxt.s32 r0, r1, q2, q3
vmlaldavxe.s16 r0, r1, q2, q3
vmlaldavaxt.s16 r0, r1, q2, q3
vmlaldavaxe.s32 r0, r1, q2, q3
