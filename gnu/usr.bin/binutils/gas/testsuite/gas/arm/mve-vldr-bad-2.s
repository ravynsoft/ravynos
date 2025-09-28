.macro cond mnem, size
.irp cond, eq, ne, gt, ge, lt, le
it \cond
\mnem\().u\size q0, [q1, #8]
.endr
.endm

.syntax unified
.thumb
vldrw.u16 q0, [q1, #4]
vldrw.u64 q0, [q1, #-4]
vldrw.u32 q0, [q1, #1]
vldrw.u32 q0, [q1, #2]
vldrw.u32 q0, [q1, #231]
vldrw.u32 q0, [q1, #516]
vldrw.u32 q0, [q1, #-516]
vldrw.u32 q0, [q0, #4]
cond vldrw, 32
it eq
vldrweq.u32 q0, [q1]
vldrweq.u32 q0, [q1]
vpst
vldrweq.u32 q0, [q1]
vldrwt.u32 q0, [q1]
vpst
vldrw.u32 q0, [q1]
vldrd.u16 q0, [q1, #8]
vldrd.u32 q0, [q1, #-8]
vldrd.u64 q0, [q1, #1]
vldrd.u64 q0, [q1, #4]
vldrd.u64 q0, [q1, #7]
vldrd.u64 q0, [q1, #228]
vldrd.u64 q0, [q1, #1024]
vldrd.u64 q0, [q1, #-1024]
vldrd.u64 q0, [q0, #8]
cond vldrd, 64
it eq
vldrdeq.u64 q0, [q1]
vldrdeq.u64 q0, [q1]
vpst
vldrdeq.u64 q0, [q1]
vldrdt.u64 q0, [q1]
vpst
vldrd.u64 q0, [q1]
