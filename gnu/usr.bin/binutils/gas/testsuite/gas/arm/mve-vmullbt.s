.syntax unified
.thumb

.macro helper_q0 op
.irp op2, q1, q2, q4, q7
.irp op3, q1, q2, q4, q7
\op q0, \op2, \op3
.endr
.endr
.endm

.macro helper_q1 op
.irp op2, q0, q2, q4, q7
.irp op3, q0, q2, q4, q7
\op q1, \op2, \op3
.endr
.endr
.endm

.macro helper_q2 op
.irp op2, q0, q1, q4, q7
.irp op3, q0, q1, q4, q7
\op q2, \op2, \op3
.endr
.endr
.endm

.macro helper_q4 op
.irp op2, q0, q1, q2, q7
.irp op3, q0, q1, q2, q7
\op q4, \op2, \op3
.endr
.endr
.endm


.macro helper_q7 op
.irp op2, q0, q1, q2, q4
.irp op3, q0, q1, q2, q4
\op q7, \op2, \op3
.endr
.endr
.endm

.macro all_qqq op
helper_q0 \op
helper_q1 \op
helper_q2 \op
helper_q4 \op
helper_q7 \op
.endm

.irp data, s8, u8, s16, u16, p8, p16
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, q0, q1, q2, q4, q7
vmullb.\data \op1, \op2, \op3
vmullt.\data \op1, \op2, \op3
.endr
.endr
.endr
.endr

all_qqq vmullb.s32
all_qqq vmullb.u32
all_qqq vmullt.s32
all_qqq vmullt.u32

vpstete
vmullbt.s8 q0, q1, q2
vmullbe.s16 q1, q0, q3
vmullbt.s32 q2, q3, q4
vmullbe.u8 q3, q2, q1
vpstete
vmullbt.u16 q4, q5, q7
vmullbe.u32 q5, q4, q6
vmullbt.p8 q6, q7, q5
vmullbe.p16 q7, q6, q0

vpstete
vmulltt.s8 q0, q1, q2
vmullte.s16 q1, q0, q3
vmulltt.s32 q2, q3, q4
vmullte.u8 q3, q2, q1
vpstete
vmulltt.u16 q4, q5, q7
vmullte.u32 q5, q4, q6
vmulltt.p8 q6, q7, q5
vmullte.p16 q7, q6, q0
