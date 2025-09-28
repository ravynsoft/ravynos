.macro cond op, lastreg, size
.irp cond, eq, ne, gt, ge, lt, le
it \cond
\op\size q0, q1, \lastreg
.endr
.endm

.syntax unified
.text
.thumb
vadd.p8 q0, q1, q2
vadd.f16 q0, q1, q2
vadd.f32 q0, q1, q2
vadd.i64 q0, q1, q2
cond vadd, q2, .i32
vsub.p8 q0, q1, q2
vsub.f16 q0, q1, q2
vsub.f32 q0, q1, q2
vsub.i64 q0, q1, q2
cond vsub, q2, .i32
vadd.p8 q0, q1, r2
vadd.f16 q0, q1, r2
vadd.f32 q0, q1, r2
vadd.i64 q0, q1, r2
cond vadd, r2, .i32
vsub.p8 q0, q1, r2
vsub.f16 q0, q1, r2
vsub.f32 q0, q1, r2
vsub.i64 q0, q1, r2
cond vsub, r2, .i32
vabd.p8 q0, q1, q2
vabd.f16 q0, q1, q2
vabd.f32 q0, q1, q2
vabd.i64 q0, q1, q2
cond vabd, q2, .s32
vadd.i32 q0, q1, sp
vsub.i32 q0, q1, sp
vadd.i32 q0, q1, pc
vsub.i32 q0, q1, pc
