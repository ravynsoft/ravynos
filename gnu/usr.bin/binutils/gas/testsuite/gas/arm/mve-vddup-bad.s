.syntax unified
.thumb
vddup.s16 q0, r0, #1
vddup.u64 q0, r0, #1
vddup.u32 q0, r0, #3
vddup.u32 q0, r0, #0
vdwdup.s16 q0, r0, r1, #1
vdwdup.u64 q0, r0, r1, #1
vdwdup.u32 q0, r0, r1, #3
vdwdup.u32 q0, r0, r1, #0
vdwdup.u32 q0, r0, sp, #1
vdwdup.u32 q0, r0, pc, #1

.irp cond, eq, ne, gt, ge, lt, le

it \cond
vddup.u32 q0, r2, #1

.endr

.irp cond, eq, ne, gt, ge, lt, le

it \cond
vdwdup.u32 q0, r2, r1, #1

.endr

it eq
vddupeq.u32 q0, r0, #1
vddupeq.u32 q0, r0, #1
vpst
vddupeq.u32 q0, r0, #1
vddupt.u32 q0, r0, #1
vpst
vddup.u32 q0, r0, #1
it eq
vdwdupeq.u32 q0, r0, r1, #1
vdwdupeq.u32 q0, r0, r1, #1
vpst
vdwdupeq.u32 q0, r0, r1, #1
vdwdupt.u32 q0, r0, r1, #1
vpst
vdwdup.u32 q0, r0, r1, #1
