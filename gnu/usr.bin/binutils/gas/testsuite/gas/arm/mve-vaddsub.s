.syntax unified
.thumb
.macro all_qqq op
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, q0, q1, q2, q4, q7
\op \op1, \op2, \op3
.endr
.endr
.endr
.endm

.macro all_qqr op
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
.irp op3, r0, r1, r2, r4, r7, r8, r10, r12, r14
\op \op1, \op2, \op3
.endr
.endr
.endr
.endm

all_qqq vadd.i8
all_qqq vadd.i16
all_qqq vadd.i32
all_qqq vadd.f16
all_qqq vadd.f32

vpstt
vaddt.i8 q0, q1, q2
vaddt.i16 q1, q2, q4
vpstee
vaddt.i32 q2, q4, q5
vadde.f16 q0, q4, q6
vadde.f32 q4, q5, q7

all_qqq vsub.i8
all_qqq vsub.i16
all_qqq vsub.i32
all_qqq vsub.f16
all_qqq vsub.f32

vpste
vsubt.i8 q0, q1, q2
vsube.i16 q1, q2, q4
vpstte
vsubt.i32 q2, q7, q5
vsubt.f16 q1, q4, q6
vsube.f32 q4, q5, q7

all_qqr vadd.i8
all_qqr vadd.i16
all_qqr vadd.i32
all_qqr vadd.f16
all_qqr vadd.f32

vpstt
vaddt.i8 q0, q1, r10
vaddt.i16 q1, q2, r12
vpstee
vaddt.i32 q2, q4, r5
vadde.f16 q5, q4, r6
vadde.f32 q4, q5, r7

all_qqr vsub.i8
all_qqr vsub.i16
all_qqr vsub.i32
all_qqr vsub.f16
all_qqr vsub.f32

vpste
vsubt.i8 q0, q1, r10
vsube.i16 q1, q2, r11
vpstte
vsubt.i32 q2, q0, r5
vsubt.f16 q1, q4, r6
vsube.f32 q4, q5, r7
