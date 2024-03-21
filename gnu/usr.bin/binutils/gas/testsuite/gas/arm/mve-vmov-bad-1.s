.syntax unified
.thumb
vmov.8 q0[0], sp
vmov.8 q0[0], pc
vmov.64 q0[0], r0
vmov.8 q0[16], r0
vmov.16 q0[8], r0
vmov.32 q0[4], r0
vpst
vmovt.8 q0[0], r0
vmovt.8 q0[0], r0
it eq
vmov.8 q0[0], r0
vmov.u8 sp, q0[0]
vmov.u8 pc, q0[0]
vmov.u64 r0, q0[0]
vmov.s64 r0, q0[0]
vmov.64 r0, q0[0]
vmov.8 r0, q0[0]
vmov.16 r0, q0[0]
vmov.f16 r0, q0[0]
vmov.u8 r0, q0[16]
vmov.u16 r0, q0[8]
vmov.32 r0, q0[4]
vpst
vmovt.u8 r0, q0[0]
vmovt.u8 r0, q0[0]
it eq
vmov.u8 r0, q0[0]
