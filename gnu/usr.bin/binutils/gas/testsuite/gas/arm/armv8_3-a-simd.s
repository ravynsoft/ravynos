	.text

A1:
	.arm

	vcadd.f32 q1,q2,q3,#90
	vcadd.f32 q1,q2,q3,#270
	vcadd.f16 d21,d22,d23,#90
	vcadd.f16 q1,q2,q3,#90
	vcadd.f32 d21,d22,d23,#90

	vcmla.f32 q1,q2,q3,#0
	vcmla.f32 q1,q2,q3,#90
	vcmla.f32 q1,q2,q3,#180
	vcmla.f32 q1,q2,q3,#270
	vcmla.f16 d21,d22,d23,#90
	vcmla.f16 q1,q2,q3,#90
	vcmla.f32 d21,d22,d23,#90

	vcmla.f16 d21,d22,d3[0],#90
	vcmla.f16 d21,d22,d3[1],#90
	vcmla.f16 q1,q2,d3[0],#90
	vcmla.f16 q1,q2,d3[1],#90
	vcmla.f32 d21,d22,d23[0],#90
	vcmla.f32 q1,q2,d23[0],#90

	vcmla.f16 q1,q2,d3[1],#0
	vcmla.f16 q1,q2,d3[1],#180
	vcmla.f16 q1,q2,d3[1],#270
	vcmla.f32 q1,q2,d3[0],#0
	vcmla.f32 q1,q2,d3[0],#180
	vcmla.f32 q1,q2,d3[0],#270

T1:
	.thumb

	vcadd.f32 q1,q2,q3,#90
	vcadd.f32 q1,q2,q3,#270
	vcadd.f16 d21,d22,d23,#90
	vcadd.f16 q1,q2,q3,#90
	vcadd.f32 d21,d22,d23,#90

	vcmla.f32 q1,q2,q3,#0
	vcmla.f32 q1,q2,q3,#90
	vcmla.f32 q1,q2,q3,#180
	vcmla.f32 q1,q2,q3,#270
	vcmla.f16 d21,d22,d23,#90
	vcmla.f16 q1,q2,q3,#90
	vcmla.f32 d21,d22,d23,#90

	vcmla.f16 d21,d22,d3[0],#90
	vcmla.f16 d21,d22,d3[1],#90
	vcmla.f16 q1,q2,d3[0],#90
	vcmla.f16 q1,q2,d3[1],#90
	vcmla.f32 d21,d22,d23[0],#90
	vcmla.f32 q1,q2,d23[0],#90

	vcmla.f16 q1,q2,d3[1],#0
	vcmla.f16 q1,q2,d3[1],#180
	vcmla.f16 q1,q2,d3[1],#270
	vcmla.f32 q1,q2,d3[0],#0
	vcmla.f32 q1,q2,d3[0],#180
	vcmla.f32 q1,q2,d3[0],#270
