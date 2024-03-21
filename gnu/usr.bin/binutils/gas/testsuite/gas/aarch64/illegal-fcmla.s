// Test illegal ARMv8.3 FCMLA and FCADD instructions with -march=armv8.3-a.
.text

	// Good.
	fcmla v0.4s, v1.4s, v2.s[0], #90
	fcmla v0.4s, v1.4s, v2.4s, #90
	fcadd v0.4h, v1.4h, v2.4h, #90

	// Bad.
	fcmla v0.4s, v1.4s, v2.s[0], #-90
	fcmla v0.4s, v1.4s, v2.s[0], #30
	fcmla v0.4s, v1.4s, v2.s[0], #360
	fcmla v0.4h, v1.4h, v2.h[2], #90
	fcmla v0.8h, v1.8h, v2.h[4], #90
	fcmla v0.4s, v1.4s, v2.s[2], #90
	fcmla v0.2s, v1.2s, v2.s[0], #90
	fcmla v0.4s, v1.4s, v2.d[0], #90
	fcmla v0.2d, v1.2d, v2.d[0], #0
	fcmla v0.4s, v1.4s, v2.4s, #-90
	fcmla v0.4s, v1.4s, v2.4s, #30
	fcmla v0.4s, v1.4s, v2.4s, #360
	fcmla v0.8s, v1.8s, v2.8s, #0
	fcmla v0.1d, v1.1d, v2.1d, #0
	fcadd v0.4h, v1.4h, v2.4h, #0
	fcadd v0.4h, v1.4h, v2.4h, #180
