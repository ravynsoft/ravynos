.syntax unified
.thumb

.macro all_vstr op, imm
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
\op \op1, [\op2, #\imm]
\op \op1, [\op2, #-\imm]
\op \op1, [\op2, #\imm]!
\op \op1, [\op2, #-\imm]!
.endr
.endr
.endm

.irp data, .32, .u32, .s32, .f32
.irp imm, 0, 4, 8, 16, 32, 64, 128, 256, 508, 340, 168, 60, 480
all_vstr vstrw\data, \imm
.endr
.endr

.irp data, .64, .u64, .s64
.irp imm, 0, 8, 16, 32, 64, 128, 256, 512, 1016, 680, 336, 960, 120
all_vstr vstrd\data, \imm
.endr
.endr


vpstete
vstrwt.32 q0, [q1, #4]
vstrwe.u32 q1, [q0, #-4]
vstrwt.s32 q2, [q2]
vstrwe.f32 q3, [q4, #-508]
vpstet
vstrdt.64 q4, [q5, #512]
vstrde.u64 q5, [q6, #1016]
vstrdt.s64 q6, [q7, #-1016]

.macro all_vldr_qq op, op1, op2, imm
\op \op1, [\op2, #\imm]
\op \op1, [\op2, #-\imm]
\op \op1, [\op2, #\imm]!
\op \op1, [\op2, #-\imm]!
.endm

.macro all_vldr_q0 op, imm
.irp op2, q1, q2, q4, q7
all_vldr_qq \op, q0, \op2, \imm
.endr
.endm

.macro all_vldr_q1 op, imm
.irp op2, q0, q2, q4, q7
all_vldr_qq \op, q1, \op2, \imm
.endr
.endm

.macro all_vldr_q2 op, imm
.irp op2, q0, q1, q4, q7
all_vldr_qq \op, q2, \op2, \imm
.endr
.endm

.macro all_vldr_q4 op, imm
.irp op2, q0, q1, q2, q7
all_vldr_qq \op, q4, \op2, \imm
.endr
.endm

.macro all_vldr_q7 op, imm
.irp op2, q0, q1, q2, q4
all_vldr_qq \op, q7, \op2, \imm
.endr
.endm

.macro all_vldr op, imm
all_vldr_q0 \op, \imm
all_vldr_q1 \op, \imm
all_vldr_q2 \op, \imm
all_vldr_q4 \op, \imm
all_vldr_q7 \op, \imm
.endm

.irp data, .32, .u32, .s32, .f32
.irp imm, 0, 4, 8, 16, 32, 64, 128, 256, 508, 340, 168, 60, 480
all_vldr vldrw\data, \imm
.endr
.endr

.irp data, .64, .u64, .s64
.irp imm, 0, 8, 16, 32, 64, 128, 256, 512, 1016, 680, 336, 960, 120
all_vldr vldrd\data, \imm
.endr
.endr

vpstete
vldrwt.32 q0, [q1, #4]
vldrwe.u32 q1, [q0, #-4]
vldrwt.s32 q2, [q3]
vldrwe.f32 q3, [q4, #-508]
vpstet
vldrdt.64 q4, [q5, #512]
vldrde.u64 q5, [q6, #1016]
vldrdt.s64 q6, [q7, #-1016]
