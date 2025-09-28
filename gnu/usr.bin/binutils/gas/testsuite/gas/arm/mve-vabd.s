.syntax unified

.macro all_qqq op
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, q0, q1, q2, q4, q7
\op \op1, \op2, \op3
.endr
.endr
.endr
.endm

all_qqq vabd.u8
all_qqq vabd.s8
all_qqq vabd.u16
all_qqq vabd.s16
all_qqq vabd.u32
all_qqq vabd.s32
all_qqq vabd.f16
all_qqq vabd.f32

vabd.f32 q4, q5, q6
vpstett
vabdt.s8 q0, q1, q2
vabde.u8 q1, q2, q0
vabdt.s16 q2, q7, q4
vabdt.f16 q6, q4, q5
vpstete
vabdt.u16 q2, q4, q5
vabde.s32 q4, q5, q6
vabdt.u32 q5, q6, q7
vabde.f32 q4, q5, q6
