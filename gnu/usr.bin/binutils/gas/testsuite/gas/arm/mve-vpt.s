.syntax unified
.thumb

.macro ins_2 cond2
vaddt.i32 q0, q1, q2
vadd\cond2\().i32 q0, q1, q2
.endm
.macro ins_3 cond2, cond3
ins_2 \cond2
vadd\cond3\().i32 q0, q1, q2
.endm
.macro ins_4 cond2, cond3, cond4
ins_3 \cond2, \cond3
vadd\cond4\().i32 q0, q1, q2
.endm

.macro vpt_1 data, cond, op1, op2
vpt\data \cond, \op1, \op2
vaddt.i32 q0, q1, q2
.endm

.macro help mask, data, cond, op1, op2
vpt\mask\data \cond, \op1, \op2
.endm

.macro vpt_2 data, cond, op1, op2
.irp cond2, t, e
help \cond2, \data, \cond, \op1, \op2
ins_2 \cond2
.endr
.endm

.macro vpt_3 data, cond, op1, op2
.irp cond2, t, e
.irp cond3, t, e
help \cond2\cond3, \data, \cond, \op1, \op2
ins_3 \cond2, \cond3
.endr
.endr
.endm

.macro vpt_4 data, cond, op1, op2
.irp cond2, t, e
.irp cond3, t, e
.irp cond4, t, e
help \cond2\cond3\cond4, \data, \cond, \op1, \op2
ins_4 \cond2, \cond3, \cond4
.endr
.endr
.endr
.endm

.macro vpt_qq data, cond
.irp op1, q0, q1, q4, q7
.irp op2, q0, q2, q5,  q7
vpt_1 \data, \cond, \op1, \op2
vpt_2 \data, \cond, \op1, \op2
vpt_3 \data, \cond, \op1, \op2
vpt_4 \data, \cond, \op1, \op2
.endr
.endr
.endm

.macro vpt_qr data, cond
.irp op1, q0, q1, q4, q7
.irp op2, r0, r1, r2, r4, r7, r8, r9, r10, r12, r14, zr
vpt_1 \data, \cond, \op1, \op2
vpt_2 \data, \cond, \op1, \op2
vpt_3 \data, \cond, \op1, \op2
vpt_4 \data, \cond, \op1, \op2
.endr
.endr
.endm

.irp data, .f16, .f32
.irp cond, eq, ne, gt, le, ge, lt
vpt_qq \data, \cond
vpt_qr \data, \cond
.endr
.endr

.irp data, .i8, .i16, .i32
.irp cond, eq, ne
vpt_qq \data, \cond
vpt_qr \data, \cond
.endr
.endr

.irp data, .u8, .u16, .u32
.irp cond, cs, hi
vpt_qq \data, \cond
vpt_qr \data, \cond
.endr
.endr

.irp data, .s8, .s16, .s32
.irp cond, ge, lt, gt, le
vpt_qq \data, \cond
vpt_qr \data, \cond
.endr
.endr

vpst
vaddt.i32 q0, q1, q2

.irp cond2, t, e
vpst\cond2
ins_2 \cond2
.irp cond3, t, e
vpst\cond2\cond3
ins_3 \cond2, \cond3
.irp cond4, t, e
vpst\cond2\cond3\cond4
ins_4 \cond2, \cond3, \cond4
.endr
.endr
.endr
