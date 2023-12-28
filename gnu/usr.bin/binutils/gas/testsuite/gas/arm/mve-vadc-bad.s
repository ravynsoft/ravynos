.syntax unified
.thumb

.irp cond, eq, ne, gt, ge, lt, le
.irp mnem, vadc.i32, vadci.i32

it \cond
\mnem q0, q1, q2

.endr
.endr

vadc.i8 q0, q1, q2
vadc.i16 q0, q1, q2
vadc.i64 q0, q1, q2
vadc.f32 q0, q1, q2
vadci.i8 q0, q1, q2
vadci.i16 q0, q1, q2
vadci.i64 q0, q1, q2
vadci.f32 q0, q1, q2
it eq
vadceq.i32 q0, q1, q2
vadceq.i32 q0, q1, q2
vpst
vadceq.i32 q0, q1, q2
vadct.i32 q0, q1, q2
vpst
vadc.i32 q0, q1, q2
it eq
vadcieq.i32 q0, q1, q2
vadcieq.i32 q0, q1, q2
vpst
vadcieq.i32 q0, q1, q2
vadcit.i32 q0, q1, q2
vpst
vadci.i32 q0, q1, q2
