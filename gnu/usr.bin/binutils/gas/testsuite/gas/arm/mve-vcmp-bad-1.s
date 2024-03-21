.syntax unified
.thumb

.irp cond, eq, ne, gt, ge, lt, le
.irp size, .
it \cond
vcmp.s32 gt, q0, q1
.endr
.endr

.irp cond, eq, ne, gt, ge, lt, le
.irp size, .
it \cond
vcmp.i16 eq, q0, r1
.endr
.endr

vcmp.f32 eq, q0, q1
vcmp.f32 eq, q0, r1
vcmp.i64 eq, q0, q1
vcmp.s32 eq, q0, q1
vcmp.s16 cs, q0, q1
vcmp.u8 le, q0, q1
vcmp.s16 q0, q1
vcmp.i32 eq, q0, sp
it eq
vcmpeq.i32 eq, q0, q1
vcmpeq.i32 eq, q0, q1
vpst
vcmpeq.i32 eq, q0, q1
vcmpt.i32 eq, q0, q1
vpst
vcmp.i32 eq, q0, q1
it eq
vcmpeq.i32 eq, q0, r1
vcmpeq.i32 eq, q0, r1
vpst
vcmpeq.i32 eq, q0, r1
vcmpt.i32 eq, q0, r1
vpst
vcmp.i32 eq, q0, r1
