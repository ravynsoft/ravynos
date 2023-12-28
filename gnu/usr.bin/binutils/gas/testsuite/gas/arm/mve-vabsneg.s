.syntax unified

.macro all_qq op
.irp op1, q0, q1, q2, q4, q7
.irp op2, q0, q1, q2, q4, q7
\op \op1, \op2
.endr
.endr
.endm
all_qq vabs.s8
all_qq vabs.s16
all_qq vabs.s32
all_qq vabs.f16
all_qq vabs.f32

vpstte
vabst.s8 q0, q1
vabst.s16 q1, q4
vabse.s32 q2, q5
vpste
vabst.f16 q0, q4
vabse.f32 q7, q5

all_qq vneg.s8
all_qq vneg.s16
all_qq vneg.s32
all_qq vneg.f16
all_qq vneg.f32

vpstee
vnegt.s8 q0, q1
vnege.s16 q1, q6
vnege.s32 q2, q5
vpste
vnegt.f16 q1, q4
vnege.f32 q7, q5
