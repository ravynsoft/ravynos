.syntax unified
.thumb
.irp data, s8, s16
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, q0, q1, q2, q4, q7
.irp rot, #90, #270
vhcadd.\data \op1, \op2, \op3, \rot
.endr
.endr
.endr
.endr
.endr

.macro vhcadd_q0 op2, rot
.irp op3, q1, q2, q4, q7
vhcadd.s32 q0, \op2, \op3, \rot
.endr
.endm

.macro vhcadd_q1 op2, rot
.irp op3, q0, q2, q4, q7
vhcadd.s32 q1, \op2, \op3, \rot
.endr
.endm

.macro vhcadd_q2 op2, rot
.irp op3, q0, q1, q4, q7
vhcadd.s32 q2, \op2, \op3, \rot
.endr
.endm

.macro vhcadd_q4 op2, rot
.irp op3, q0, q1, q2, q7
vhcadd.s32 q4, \op2, \op3, \rot
.endr
.endm

.macro vhcadd_q7 op2, rot
.irp op3, q0, q1, q2, q4
vhcadd.s32 q7, \op2, \op3, \rot
.endr
.endm

.irp op2, q0, q1, q2, q4, q7
.irp rot, #90, #270
vhcadd_q0 \op2, \rot
vhcadd_q1 \op2, \rot
vhcadd_q2 \op2, \rot
vhcadd_q4 \op2, \rot
vhcadd_q7 \op2, \rot
.endr
.endr

vpste
vhcaddt.s8 q0, q1, q2, #90
vhcadde.s16 q0, q1, q2, #270
