.syntax unified
.thumb

.macro vcmp_qq data, cond
.irp op1, q0, q1, q4, q7
.irp op2, q0, q2, q5, q7
vcmp\data \cond, \op1, \op2
.endr
.endr
.endm

.macro vcmp_qr data, cond
.irp op1, q0, q1, q4, q7
.irp op2, r0, r2, r5, r7, r8, r11, r14, zr
vcmp\data \cond, \op1, \op2
.endr
.endr
.endm

.irp data, .f16, .f32
.irp cond, eq, ne, gt, lt, le, ge
vcmp_qq \data, \cond
vcmp_qr \data, \cond
.endr
.endr

.irp data, .i8, .i16, .i32
.irp cond, eq, ne
vcmp_qq \data, \cond
vcmp_qr \data, \cond
.endr
.endr

.irp data, .u8, .u16, .u32
.irp cond,  cs, hi
vcmp_qq \data, \cond
vcmp_qr \data, \cond
.endr
.endr

.irp data, .s8, .s16, .s32
.irp cond, ge, lt, gt, le
vcmp_qq \data, \cond
vcmp_qr \data, \cond
.endr
.endr

vpstete
vcmpt.f32 eq, q0, q1
vcmpe.f16 eq, q0, q1
vcmpt.i32 eq, q0, q1
vcmpe.s16 ge, q0, q1

vpstete
vcmpt.f32 eq, q0, r1
vcmpe.f16 eq, q0, r1
vcmpt.i32 eq, q0, r1
vcmpe.u16 cs, q0, r1

