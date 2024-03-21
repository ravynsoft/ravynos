.syntax unified
.thumb

.irp op0, s0, s1, s2, s4, s8, s16, s30, s31
.irp op1, r0, r1, r2, r4, r7, r8, r10, r12, r14
vmov \op0, \op1
vmov \op1, \op0
.endr
.endr

.macro vmov_rr, op0, op1
.irp op2, d0, d1, d2, d4, d8, d15
vmov \op0, \op1, \op2
vmov \op2, \op0, \op1
.endr
vmov \op0, \op1, s0, s1
vmov \op0, \op1, s1, s2
vmov \op0, \op1, s2, s3
vmov \op0, \op1, s4, s5
vmov \op0, \op1, s8, s9
vmov \op0, \op1, s16, s17
vmov \op0, \op1, s30, s31
vmov s0, s1, \op0, \op1
vmov s1, s2, \op0, \op1
vmov s2, s3, \op0, \op1
vmov s4, s5, \op0, \op1
vmov s8, s9, \op0, \op1
vmov s16, s17, \op0, \op1
vmov s30, s31, \op0, \op1
.endm

.irp op0, r1, r2, r4, r7, r8, r10, r12, r14
vmov_rr r0, \op0
.endr

.irp op0, r0, r2, r4, r7, r8, r10, r12, r14
vmov_rr r1, \op0
.endr

.irp op0, r0, r1, r4, r7, r8, r10, r12, r14
vmov_rr r2, \op0
.endr

.irp op0, r0, r1, r2, r7, r8, r10, r12, r14
vmov_rr r4, \op0
.endr

.irp op0, r0, r1, r2, r4, r8, r10, r12, r14
vmov_rr r7, \op0
.endr

.irp op0, r0, r1, r2, r4, r7, r10, r12, r14
vmov_rr r8, \op0
.endr

.irp op0, r0, r1, r2, r4, r7, r8, r12, r14
vmov_rr r10, \op0
.endr

.irp op0, r0, r1, r2, r4, r7, r8, r10, r14
vmov_rr r12, \op0
.endr

.irp op0, r0, r1, r2, r4, r7, r8, r10, r12
vmov_rr r14, \op0
.endr

.macro vmov_qidx_r size, idx
.irp op1, q0, q1, q2, q4, q7
.irp op2, r0, r1, r2, r4, r7, r8, r10, r12, r14
vmov\size \op1[\idx], \op2
.endr
.endr
.endm

.irp idx, 0, 1, 2, 4, 8, 15, 13, 6
vmov_qidx_r .8, \idx
.endr

.irp idx, 0, 1, 2, 4, 7
vmov_qidx_r .16, \idx
.endr

.irp idx, 0, 1, 2, 3
vmov_qidx_r .32, \idx
.endr

.macro vmov_r_qidx size, idx
.irp op1, r0, r1, r2, r4, r7, r8, r10, r12, r14
.irp op2, q0, q1, q2, q4, q7
vmov\size \op1, \op2[\idx]
.endr
.endr
.endm

.irp data, .u8, .s8
.irp idx, 0, 1, 2, 4, 8, 15, 13, 6
vmov_r_qidx \data, \idx
.endr
.endr

.irp data, .u16, .s16
.irp idx, 0, 1, 2, 4, 7
vmov_r_qidx \data, \idx
.endr
.endr

.irp idx, 0, 1, 2, 3
vmov_r_qidx .32, \idx
.endr

vmov.i32 q0, #0
vmov.i32 q0, #255        @ 0x000000FF 000000FF
vmov.i32 q0, #65280      @ 0x0000FF00 0000FF00
vmov.i32 q0, #4278190080 @ 0xFF000000 FF000000
vmov.i32 q0, #16711680   @ 0x00FF0000 00FF0000

vmov.i16 q0, #0
vmov.i16 q0, #255      @ 0x00FF 00FF 00FF 00FF
vmov.i16 q0, #65280    @ 0xFF00 FF00 FF00 FF00

vmov.i8 q0, #0
vmov.i8 q0, #255      @ 0xFF FF FF FF FF FF FF FF

vmov.i64 q0, #18374686479671623680 @ 0xFF00000000000000
vmov.i64 q0, #71776119061217280 @ 0x00FF000000000000
vmov.i64 q0, #280375465082880 @ 0x0000FF0000000000
vmov.i64 q0, #1095216660480 @ 0x000000FF00000000
vmov.i64 q0, #4278190080 @ 0x00000000FF000000
vmov.i64 q0, #16711680 @ 0x00000000000FF0000
vmov.i64 q0, #65280 @ 0x0000000000000FF00
vmov.i64 q0, #255 @ 0x000000000000000FF

.irp op0, s0, s1, s2, s4, s8, s16, s30, s31
.irp op1, r0, r1, r2, r4, r7, r8, r10, r12, r14
vmov.f16 \op0, \op1
vmov.f16 \op1, \op0
.endr
.endr

vmov.f32 q0, #0.0
vmov.f32 q0, #-31.0   @  0xC1F80000 C1F80000
vmov.f32 q0, #-31.0   @  0xC1F80000 C1F80000
vmov.f32 q0, #-1.9375 @  0xBFF80000 BFF80000
vmov.f32 q0, #1.0     @  0x3F800000 3F800000
vmov.f16 s0, #2.0
vmov.f32 s0, #2.5
