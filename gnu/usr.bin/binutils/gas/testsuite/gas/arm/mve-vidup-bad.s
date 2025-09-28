.syntax unified
.thumb
vidup.s16 q0, r0, #1
vidup.u64 q0, r0, #1
vidup.u32 q0, r0, #3
vidup.u32 q0, r0, #0
viwdup.s16 q0, r0, r1, #1
viwdup.u64 q0, r0, r1, #1
viwdup.u32 q0, r0, r1, #3
viwdup.u32 q0, r0, r1, #0
viwdup.u32 q0, r0, sp, #1
viwdup.u32 q0, r0, pc, #1

.irp cond, eq, ne, gt, ge, lt, le

it \cond
vidup.u32 q0, r2, #1

.endr

.irp cond, eq, ne, gt, ge, lt, le

it \cond
viwdup.u32 q0, r2, r1, #1

.endr

it eq
vidupeq.u32 q0, r0, #1
vidupeq.u32 q0, r0, #1
vpst
vidupeq.u32 q0, r0, #1
vidupt.u32 q0, r0, #1
vpst
vidup.u32 q0, r0, #1
it eq
viwdupeq.u32 q0, r0, r1, #1
viwdupeq.u32 q0, r0, r1, #1
vpst
viwdupeq.u32 q0, r0, r1, #1
viwdupt.u32 q0, r0, r1, #1
vpst
viwdup.u32 q0, r0, r1, #1
