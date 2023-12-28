.syntax unified
.thumb

.macro n_vstr_w_vldr op, imm
.irp op1, q0, q1, q2, q4, q7
.irp op2, r0, r1, r2, r4, r7
\op \op1, [\op2, #\imm]
\op \op1, [\op2, #-\imm]
\op \op1, [\op2, #\imm]!
\op \op1, [\op2, #-\imm]!
\op \op1, [\op2], #\imm
\op \op1, [\op2], #-\imm
.endr
.endr
.endm

.irp mnem, vstrb.16, vstrb.32
.irp imm, 0, 1, 2, 4, 8, 16, 32, 64, 127, 120, 15
n_vstr_w_vldr \mnem, \imm
.endr
.endr

.irp imm, 0, 2, 4, 8, 16, 32, 64, 128, 254, 240, 30
n_vstr_w_vldr vstrh.32, \imm
.endr

.macro wb_same_size_vstr_vldr  op, imm
.irp op1, q0, q1, q2, q4, q7
.irp op2, r0, r1, r2, r4, r7, r8, r10, r12, r14
\op \op1, [\op2, #\imm]!
\op \op1, [\op2, #-\imm]!
\op \op1, [\op2], #\imm
\op \op1, [\op2], #-\imm
.endr
.endr
.endm

.macro no_wb_same_size_vstr_vldr  op, imm
.irp op1, q0, q1, q2, q4, q7
.irp op2, r0, r1, r2, r4, r7, r8, r10, r12, r13, r14
\op \op1, [\op2, #\imm]
\op \op1, [\op2, #-\imm]
.endr
.endr
.endm

.irp mnem, vstrb.8, vstrb.s8, vstrb.u8
.irp imm, 0, 1, 2, 4, 8, 16, 32, 64, 127, 120, 15
wb_same_size_vstr_vldr \mnem, \imm
no_wb_same_size_vstr_vldr \mnem, \imm
.endr
.endr

.irp mnem, vstrh.16, vstrh.s16, vstrh.u16, vstrh.f16
.irp imm, 0, 2, 4, 8, 16, 32, 64, 128, 254, 240, 30
wb_same_size_vstr_vldr \mnem, \imm
no_wb_same_size_vstr_vldr \mnem, \imm
.endr
.endr

.irp mnem, vstrw.32, vstrw.s32, vstrw.u32, vstrw.f32
.irp imm, 0, 4, 8, 16, 32, 64, 128, 256, 508, 480, 60
wb_same_size_vstr_vldr \mnem, \imm
no_wb_same_size_vstr_vldr \mnem, \imm
.endr
.endr

vpstet
vstrbt.u8 q0, [r12, #1]
vstrbe.16 q3, [r2, #-127]
vstrbt.32 q3, [r3, #-93]!
vpstete
vstrht.f16 q5, [r4], #254
vstrhe.32 q5, [r0, #126]
vstrwt.32  q3, [r5, #508]
vstrwe.u32 q5, [r8], #244


.irp mnem, vldrb.s16, vldrb.u16, vldrb.s32, vldrb.u32
.irp imm, 0, 1, 2, 4, 8, 16, 32, 64, 127, 120, 15
n_vstr_w_vldr \mnem, \imm
.endr
.endr

.irp mnem, vldrh.s32, vldrh.u32
.irp imm, 0, 2, 4, 8, 16, 32, 64, 128, 254, 240, 30
n_vstr_w_vldr \mnem, \imm
.endr
.endr

.irp mnem, vldrb.8, vldrb.s8, vldrb.u8
.irp imm, 0, 1, 2, 4, 8, 16, 32, 64, 127, 120, 15
wb_same_size_vstr_vldr \mnem, \imm
no_wb_same_size_vstr_vldr \mnem, \imm
.endr
.endr

.irp mnem, vldrh.16, vldrh.s16, vldrh.u16, vldrh.f16
.irp imm, 0, 2, 4, 8, 16, 32, 64, 128, 254, 240, 30
wb_same_size_vstr_vldr \mnem, \imm
no_wb_same_size_vstr_vldr \mnem, \imm
.endr
.endr

.irp mnem, vldrw.32, vldrw.s32, vldrw.u32, vldrw.f32
.irp imm, 0, 4, 8, 16, 32, 64, 128, 256, 508, 480, 60
wb_same_size_vstr_vldr \mnem, \imm
no_wb_same_size_vstr_vldr \mnem, \imm
.endr
.endr
vpstet
vldrbt.u8 q0, [r12, #1]
vldrbe.u16 q3, [r2, #-127]
vldrbt.u32 q3, [r3, #-93]!
vpstete
vldrht.f16 q5, [r4], #254
vldrhe.s32 q5, [r0, #126]
vldrwt.32  q3, [r5, #508]
vldrwe.u32 q5, [r8], #244
