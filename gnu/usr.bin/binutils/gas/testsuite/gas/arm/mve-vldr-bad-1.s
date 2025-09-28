.macro cond mnem
.irp cond, eq, ne, gt, ge, lt, le
it \cond
\mnem\().u32 q0, [r0, q1]
.endr
.endm

.syntax unified
.thumb
vldrb.16 q0, [r0, q1]
vldrb.p16 q0, [r0, q1]
vldrb.f16 q0, [r0, q1]
vldrb.32 q0, [r0, q1]
vldrb.f32 q0, [r0, q1]
vldrb.64 q0, [r0, q1]
vldrb.u64 q0, [r0, q1]
vldrb.s64 q0, [r0, q1]
vldrb.u32 q0, [pc, q1]
vldrb.u32 q0, [r0, q0]
cond vldrb
it eq
vldrbeq.u32 q0, [r0, q1]
vldrbeq.u32 q0, [r0, q1]
vpst
vldrbeq.u32 q0, [r0, q1]
vldrbt.u32 q0, [r0, q1]
vpst
vldrb.u32 q0, [r0, q1]

vldrh.32 q0, [r0, q1]
vldrh.f32 q0, [r0, q1]
vldrh.64 q0, [r0, q1]
vldrh.u64 q0, [r0, q1]
vldrh.s64 q0, [r0, q1]
vldrh.u32 q0, [pc, q1]
vldrh.u32 q0, [r0, q0]
cond vldrh
it eq
vldrheq.u32 q0, [r0, q1]
vldrheq.u32 q0, [r0, q1]
vpst
vldrheq.u32 q0, [r0, q1]
vldrht.u32 q0, [r0, q1]
vpst
vldrh.u32 q0, [r0, q1]

vldrw.64 q0, [r0, q1]
vldrw.u64 q0, [r0, q1]
vldrw.s64 q0, [r0, q1]
vldrw.u32 q0, [pc, q1]
vldrw.u32 q0, [r0, q0]
cond vldrw
it eq
vldrweq.u32 q0, [r0, q1]
vldrweq.u32 q0, [r0, q1]
vpst
vldrweq.u32 q0, [r0, q1]
vldrwt.u32 q0, [r0, q1]
vpst
vldrw.u32 q0, [r0, q1]

.macro cond64
.irp cond, eq, ne, gt, ge, lt, le
it \cond
vldrd.u64 q0, [r0, q1]
.endr
.endm

vldrd.8 q0, [r0, q1]
vldrd.u8 q0, [r0, q1]
vldrd.s8 q0, [r0, q1]
vldrd.p8 q0, [r0, q1]
vldrd.16 q0, [r0, q1]
vldrd.u16 q0, [r0, q1]
vldrd.s16 q0, [r0, q1]
vldrd.p16 q0, [r0, q1]
vldrd.f16 q0, [r0, q1]
vldrd.32 q0, [r0, q1]
vldrd.u32 q0, [r0, q1]
vldrd.s32 q0, [r0, q1]
vldrd.f32 q0, [r0, q1]
cond64
it eq
vldrdeq.u64 q0, [r0, q1]
vldrdeq.u64 q0, [r0, q1]
vpst
vldrdeq.u64 q0, [r0, q1]
vldrdt.u64 q0, [r0, q1]
vpst
vldrd.u64 q0, [r0, q1]

vldrb.u8 q0, [r0, q1, #0]
vldrb.u8 q0, [r0, q1, UXTW #1]
vldrb.u16 q0, [r0, q1, UXTW #1]
vldrb.u32 q0, [r0, q1, UXTW #1]
vldrh.u16 q0, [r0, q1, #1]
vldrh.u16 q0, [r0, q1, UXTW #2]
vldrh.u32 q0, [r0, q1, UXTW #2]
vldrh.u16 q0, [r0, q1, UXTW #3]
vldrh.u32 q0, [r0, q1, UXTW #3]
vldrw.u32 q0, [r0, q1, #2]
vldrw.u32 q0, [r0, q1, UXTW #1]
vldrw.u32 q0, [r0, q1, UXTW #3]
vldrd.u64 q0, [r0, q1, #3]
vldrd.u64 q0, [r0, q1, UXTW #1]
vldrd.u64 q0, [r0, q1, UXTW #2]
vldrd.u64 q0, [r0, q1, UXTW #4]
