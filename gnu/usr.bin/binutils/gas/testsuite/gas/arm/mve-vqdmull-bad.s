.macro cond op, lastreg
.irp cond, eq, ne, gt, ge, lt, le
it \cond
\op\().s16 q0, q1, \lastreg
.endr
.endm

.syntax unified
.thumb
vqdmullt.s8 q0, q1, q2
vqdmullt.u8 q0, q1, q2
vqdmullt.i16 q0, q1, q2
vqdmullt.s64 q0, q1, q2
vqdmullb.s8 q0, q1, q2
vqdmullb.u8 q0, q1, q2
vqdmullb.i16 q0, q1, q2
vqdmullb.s64 q0, q1, q2
vqdmullt.s8 q0, q1, r2
vqdmullt.u8 q0, q1, r2
vqdmullt.i16 q0, q1, r2
vqdmullt.s64 q0, q1, r2
vqdmullb.s8 q0, q1, r2
vqdmullb.u8 q0, q1, r2
vqdmullb.i16 q0, q1, r2
vqdmullb.s64 q0, q1, r2
vqdmullt.s32 q0, q0, q2
vqdmullt.s32 q0, q1, q0
vqdmullb.s32 q0, q0, q2
vqdmullb.s32 q0, q1, q0
vqdmullt.s32 q0, q0, r2
vqdmullb.s32 q0, q0, r2
vqdmullt.s16 q0, q0, sp
vqdmullt.s16 q0, q0, pc
vqdmullb.s16 q0, q0, sp
vqdmullb.s16 q0, q0, pc
cond vqdmullt, q2
cond vqdmullb, q2
cond vqdmullt, r2
cond vqdmullb, r2
it eq
vqdmullteq.s32 q0, q1, q2
vqdmullteq.s32 q0, q1, q2
vpst
vqdmullteq.s32 q0, q1, q2
vqdmulltt.s32 q0, q1, q2
vpst
vqdmullt.s32 q0, q1, q2
it eq
vqdmullbeq.s32 q0, q1, q2
vqdmullbeq.s32 q0, q1, q2
vpst
vqdmullbeq.s32 q0, q1, q2
vqdmullbt.s32 q0, q1, q2
vpst
vqdmullb.s32 q0, q1, q2
