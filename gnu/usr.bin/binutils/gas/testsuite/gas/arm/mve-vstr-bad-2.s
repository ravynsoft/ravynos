.macro cond mnem, size
.irp cond, eq, ne, gt, ge, lt, le
it \cond
\mnem\().u\size q0, [q1, #8]
.endr
.endm

.syntax unified
.thumb
vstrw.u16 q0, [q1, #4]
vstrw.u64 q0, [q1, #-4]
vstrw.u32 q0, [q1, #1]
vstrw.u32 q0, [q1, #2]
vstrw.u32 q0, [q1, #231]
vstrw.u32 q0, [q1, #516]
vstrw.u32 q0, [q1, #-516]
cond vstrw, 32
it eq
vstrweq.u32 q0, [q1]
vstrweq.u32 q0, [q1]
vpst
vstrweq.u32 q0, [q1]
vstrwt.u32 q0, [q1]
vpst
vstrw.u32 q0, [q1]
vstrd.u16 q0, [q1, #8]
vstrd.u32 q0, [q1, #-8]
vstrd.u64 q0, [q1, #1]
vstrd.u64 q0, [q1, #4]
vstrd.u64 q0, [q1, #7]
vstrd.u64 q0, [q1, #228]
vstrd.u64 q0, [q1, #1024]
vstrd.u64 q0, [q1, #-1024]
cond vstrd, 64
it eq
vstrdeq.u64 q0, [q1]
vstrdeq.u64 q0, [q1]
vpst
vstrdeq.u64 q0, [q1]
vstrdt.u64 q0, [q1]
vpst
vstrd.u64 q0, [q1]

