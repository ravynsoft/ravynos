.macro cond mnem
.irp cond, eq, ne, gt, ge, lt, le
it \cond
\mnem\().32 q0, [r0]
.endr
.endm

.syntax unified
.thumb
vstrb.8 q0, [r0, #128]
vstrb.8 q0, [r0, #-128]
vstrb.16 q0, [r0, #128]
vstrb.16 q0, [r0, #-128]
vstrb.32 q0, [r0, #128]
vstrb.32 q0, [r0, #-128]
vstrb.8 q0, [r0, #128]!
vstrb.8 q0, [r0, #-128]!
vstrb.16 q0, [r0, #128]!
vstrb.16 q0, [r0, #-128]!
vstrb.32 q0, [r0, #128]!
vstrb.32 q0, [r0, #-128]!
vstrb.8 q0, [r0], #128
vstrb.8 q0, [r0], #-128
vstrb.16 q0, [r0], #128
vstrb.16 q0, [r0], #-128
vstrb.32 q0, [r0], #128
vstrb.32 q0, [r0], #-128
vstrb.16 q0, [r10, #2]
vstrb.16 q0, [r10, #2]!
vstrb.16 q0, [r10], #2
vstrb.8 q0, [sp, #2]!
vstrb.8 q0, [sp], #2
vstrb.8 q0, [pc, #2]
cond vstrb
vstrb.u16 q0, [r0]
vstrb.s16 q0, [r0]
vstrb.f16 q0, [r0]
vstrb.p16 q0, [r0]
vstrb.u32 q0, [r0]
vstrb.s32 q0, [r0]
vstrb.f32 q0, [r0]
vstrh.16 q0, [r0, #1]
vstrh.16 q0, [r0, #17]
vstrh.16 q0, [r0, #-17]
vstrh.16 q0, [r0, #256]
vstrh.16 q0, [r0, #-256]
vstrh.32 q0, [r0, #1]
vstrh.32 q0, [r0, #17]
vstrh.32 q0, [r0, #-17]
vstrh.32 q0, [r0, #256]
vstrh.32 q0, [r0, #-256]
vstrh.16 q0, [r0, #1]!
vstrh.16 q0, [r0, #17]!
vstrh.16 q0, [r0, #-17]!
vstrh.16 q0, [r0, #256]!
vstrh.16 q0, [r0, #-256]!
vstrh.32 q0, [r0, #1]!
vstrh.32 q0, [r0, #17]!
vstrh.32 q0, [r0, #-17]!
vstrh.32 q0, [r0, #256]!
vstrh.32 q0, [r0, #-256]!
vstrh.16 q0, [r0], #1
vstrh.16 q0, [r0], #17
vstrh.16 q0, [r0], #-17
vstrh.16 q0, [r0], #256
vstrh.16 q0, [r0], #-256
vstrh.32 q0, [r0], #1
vstrh.32 q0, [r0], #17
vstrh.32 q0, [r0], #-17
vstrh.32 q0, [r0], #256
vstrh.32 q0, [r0], #-256
vstrh.32 q0, [r10, #4]
vstrh.16 q0, [sp, #2]!
vstrh.16 q0, [sp], #2
vstrh.16 q0, [pc, #2]
cond vstrh
vstrh.8 q0, [r0]
vstrh.u8 q0, [r0]
vstrh.s8 q0, [r0]
vstrh.p8 q0, [r0]
vstrh.u32 q0, [r0]
vstrh.s32 q0, [r0]
vstrh.f32 q0, [r0]
vstrw.32 q0, [r0, #3]
vstrw.32 q0, [r0, #-3]
vstrw.32 q0, [r0, #514]
vstrw.32 q0, [r0, #-258]
vstrw.32 q0, [r0, #258]
vstrw.32 q0, [r0, #516]
vstrw.32 q0, [r0, #-516]
vstrw.32 q0, [r0, #3]!
vstrw.32 q0, [r0, #-3]!
vstrw.32 q0, [r0, #514]!
vstrw.32 q0, [r0, #-258]!
vstrw.32 q0, [r0, #258]!
vstrw.32 q0, [r0, #516]!
vstrw.32 q0, [r0, #-516]!
vstrw.32 q0, [r0], #3
vstrw.32 q0, [r0], #-3
vstrw.32 q0, [r0], #514
vstrw.32 q0, [r0], #-258
vstrw.32 q0, [r0], #258
vstrw.32 q0, [r0], #516
vstrw.32 q0, [r0], #-516
vstrw.32 q0, [sp, #4]!
vstrw.32 q0, [pc, #4]
cond vstrw
vstrw.8 q0, [r0]
vstrw.u8 q0, [r0]
vstrw.s8 q0, [r0]
vstrw.p8 q0, [r0]
vstrw.16 q0, [r0]
vstrw.u16 q0, [r0]
vstrw.s16 q0, [r0]
vstrw.f16 q0, [r0]
vstrw.p16 q0, [r0]
it eq
vstrbeq.8 q0, [r0]
vstrbeq.8 q0, [r0]
vpst
vstrbeq.8 q0, [r0]
vstrbt.8 q0, [r0]
vpst
vstrb.8 q0, [r0]
it eq
vstrheq.16 q0, [r0]
vstrheq.16 q0, [r0]
vpst
vstrheq.16 q0, [r0]
vstrht.16 q0, [r0]
vpst
vstrh.16 q0, [r0]
it eq
vstrweq.32 q0, [r0]
vstrweq.32 q0, [r0]
vpst
vstrweq.32 q0, [r0]
vstrwt.32 q0, [r0]
vpst
vstrw.32 q0, [r0]
