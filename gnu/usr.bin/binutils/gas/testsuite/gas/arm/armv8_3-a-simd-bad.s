	.text

A1:
	.arm

	vcadd d0,d1,d2,#90
	vcadd.f32 q0,q1,q2,#0
	vcadd.f32 q0,q1,q2,#180
	vcadd.f16 s0,s1,s2,#90
	vcadd.f64 d0,d1,d2,#90
	vcadd.f64 q0,q1,q2,#90

	vcmla d0,d1,d2,#90
	vcmla.f32 q0,q1,q2,#-90
	vcmla.f32 q0,q1,q2,#120
	vcmla.f32 q0,q1,q2,#360
	vcmla.f16 s0,s1,s2,#90
	vcmla.f64 d0,d1,d2,#90
	vcmla.f64 q0,q1,q2,#90

	vcmla.f16 q0,q1,q2[0],#90
	vcmla.f32 q0,q1,q2[0],#90
	vcmla.f16 d0,d1,d2[2],#90
	vcmla.f16 q0,q1,d2[2],#90
	vcmla.f16 q0,q1,d16[1],#90
	vcmla.f32 q0,q1,d2[1],#90

T1:
	.thumb

	vcadd d0,d1,d2,#90
	vcadd.f32 q0,q1,q2,#0
	vcadd.f32 q0,q1,q2,#180
	vcadd.f16 s0,s1,s2,#90
	vcadd.f64 d0,d1,d2,#90
	vcadd.f64 q0,q1,q2,#90

	vcmla d0,d1,d2,#90
	vcmla.f32 q0,q1,q2,#-90
	vcmla.f32 q0,q1,q2,#120
	vcmla.f32 q0,q1,q2,#360
	vcmla.f16 s0,s1,s2,#90
	vcmla.f64 d0,d1,d2,#90
	vcmla.f64 q0,q1,q2,#90

	vcmla.f16 q0,q1,q2[0],#90
	vcmla.f32 q0,q1,q2[0],#90
	vcmla.f16 d0,d1,d2[2],#90
	vcmla.f16 q0,q1,d2[2],#90
	vcmla.f16 q0,q1,d16[1],#90
	vcmla.f32 q0,q1,d2[1],#90
