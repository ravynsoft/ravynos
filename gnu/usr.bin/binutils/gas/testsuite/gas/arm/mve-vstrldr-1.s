.syntax unified
.thumb

.macro all_vstr op, size, ext
.irp op1, q0, q1, q2, q4, q7
.irp op2, r0, r1, r2, r4, r7, r8, r10, r12, r13, r14
.irp op3, q0, q1, q2, q4, q7
\op\()\size \op1, [\op2, \op3]
\op\()\size \op1, [\op2, \op3, uxtw #\ext]
.endr
.endr
.endr
.endm

.irp size, .8, .16, .32
all_vstr vstrb, \size, 0
.endr

.irp size, .16, .32
all_vstr vstrh, \size, 1
.endr

all_vstr vstrw, .32, 2
all_vstr vstrd, .64, 3

vpstete
vstrbt.8 q1, [r0, q0]
vstrbe.8 q1, [r2, q2]
vstrbt.16 q2, [r3, q1]
vstrbe.16 q3, [r4, q6]
vpstete
vstrbt.32 q4, [r8, q2]
vstrbe.32 q7, [sp, q6]
vstrht.16 q0, [r0, q1]
vstrhe.16 q2, [r2, q0]
vpstet
vstrht.32 q1, [r1, q7]
vstrhe.32 q3, [r3, q2]
vstrht.16 q4, [r6, q5, UXTW #1]
vpstete
vstrht.16 q6, [r10, q3, UXTW #1]
vstrhe.32 q5, [r7, q4, UXTW #1]
vstrht.32 q7, [sp, q6, UXTW #1]
vstrwe.32 q0, [r2, q1]
vpstete
vstrwt.32 q1, [r5, q7]
vstrwe.32 q2, [r8, q3, UXTW #2]
vstrwt.32 q5, [sp, q0, UXTW #2]
vstrde.64 q0, [sp, q7]
vpstte
vstrdt.64 q2, [r0, q1]
vstrdt.64 q3, [r3, q5, UXTW #3]
vstrde.64 q7, [r7, q4, UXTW #3]

.macro all_vldr op, size, ext
.irp op2, r0, r1, r2, r4, r7, r8, r10, r12, r13, r14
.irp op3, q1, q2, q4, q7
\op\()\size q0, [\op2, \op3]
\op\()\size q0, [\op2, \op3, uxtw #\ext]
.endr
.irp op3, q0, q2, q4, q7
\op\()\size q1, [\op2, \op3]
\op\()\size q1, [\op2, \op3, uxtw #\ext]
.endr
.irp op3, q0, q1, q4, q7
\op\()\size q2, [\op2, \op3]
\op\()\size q2, [\op2, \op3, uxtw #\ext]
.endr
.irp op3, q0, q1, q2, q7
\op\()\size q4, [\op2, \op3]
\op\()\size q4, [\op2, \op3, uxtw #\ext]
.endr
.irp op3, q0, q1, q2, q4
\op\()\size q7, [\op2, \op3]
\op\()\size q7, [\op2, \op3, uxtw #\ext]
.endr
.endr
.endm

.irp data, .u8, .s16, .u16, .s32, .u32
all_vldr vldrb, \data, 0
.endr

.irp data, .u16, .s32, .u32
all_vldr vldrh, \data, 1
.endr

all_vldr vldrw, .u32, 2
all_vldr vldrd, .u64, 3

vpstete
vldrbt.u8 q1, [r0, q0]
vldrbe.u8 q1, [r2, q2]
vldrbt.u16 q2, [r3, q1]
vldrbe.s16 q3, [r4, q6]
vpstete
vldrbt.u32 q4, [r8, q2]
vldrbe.s32 q7, [sp, q6]
vldrht.u16 q0, [r0, q1]
vldrhe.u16 q2, [r2, q0]
vpstete
vldrht.u32 q1, [r1, q7]
vldrhe.u32 q3, [r3, q2]
vldrht.u16 q4, [r6, q5, UXTW #1]
vldrhe.u16 q6, [r10, q3, UXTW #1]
vpstete
vldrht.u32 q5, [r7, q4, UXTW #1]
vldrhe.u32 q7, [sp, q6, UXTW #1]
vldrwt.u32 q0, [r2, q1]
vldrwe.u32 q1, [r5, q7]
vpstete
vldrwt.u32 q2, [r8, q3, UXTW #2]
vldrwe.u32 q5, [sp, q0, UXTW #2]
vldrdt.u64 q0, [sp, q7]
vldrde.u64 q2, [r0, q1]
vpste
vldrdt.u64 q3, [r3, q5, UXTW #3]
vldrde.u64 q7, [r7, q4, UXTW #3]

.irp dt, u8, s16, 8
vldrb.\dt q0, [r2, q3]
.endr
.irp dt, 16, u16, s32, f16, p16, u32
vldrh.\dt q0, [r2, q3, UXTW #1]
.endr
.irp dt, 32, u32, f32, p32
vldrw.\dt q0, [r2, q3, UXTW #2]
.endr
.irp dt, 64, u64, f64, p64
vldrd.\dt q0, [r2, q3, UXTW #3]
.endr
