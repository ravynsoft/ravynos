.syntax unified
.thumb

.macro all_rqq op
.irp op1, r0, r1, r2, r4, r7, r8, r10, r12, r14
.irp op2, q0, q1, q2, q4, q7
.irp op3, q0, q1, q2, q4, q7
\op \op1, \op2, \op3
.endr
.endr
.endr
.endm

all_rqq vabav.s8
all_rqq vabav.u8
all_rqq vabav.s16
all_rqq vabav.u16
all_rqq vabav.s32
all_rqq vabav.u32

vpstet
vabavt.s8  r0,  q0, q7
vabave.u8  r1,  q1, q0
vabavt.s16 r2,  q2, q4

vpstet
vabavt.u16 r3,  q4, q2
vabave.s32 r10, q7, q1
vabavt.u32 r14, q7, q0
