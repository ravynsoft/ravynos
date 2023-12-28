/* simdhp.s Test file for AArch64 half-precision floating-point
   vector instructions.  */

	/* Vector three-same.  */

	.macro three_same, op
	\op	v1.2d, v2.2d, v3.2d
	\op	v1.2s, v2.2s, v3.2s
	\op	v1.4s, v2.4s, v3.4s
	\op	v0.4h, v0.4h, v0.4h
	\op	v1.4h, v2.4h, v3.4h
	\op	v0.8h, v0.8h, v0.8h
	\op	v1.8h, v2.8h, v3.8h
	.endm

	.text

	three_same fmaxnm
	three_same fmaxnmp
	three_same fminnm
	three_same fminnmp
	three_same fmla
	three_same fmls
	three_same fadd
	three_same faddp
	three_same fsub
	three_same fmulx
	three_same fmul
	three_same fcmeq
	three_same fcmge
	three_same fcmgt
	three_same facge
	three_same facgt
	three_same fmax
	three_same fmaxp
	three_same fmin
	three_same fminp
	three_same frecps
	three_same fdiv
	three_same frsqrts

	/* Scalar three-same.  */

	.macro sthree_same, op
	\op	d0, d1, d2
	\op	s0, s1, s2
	\op	h0, h1, h2
	\op	h0, h0, h0
	.endm

	sthree_same fabd
	sthree_same fmulx
	sthree_same fcmeq
	sthree_same fcmgt
	sthree_same fcmge
	sthree_same facge
	sthree_same facgt
	sthree_same frecps
	sthree_same frsqrts

	/* Vector two-register misc.  */

	.macro tworeg_zero, op
	\op	v0.2d, v1.2d, #0.0
	\op	v0.2s, v1.2s, #0.0
	\op	v0.4s, v1.4s, #0.0
	\op	v0.4h, v1.4h, #0.0
	\op	v0.8h, v1.8h, #0.0
	.endm

	tworeg_zero fcmgt
	tworeg_zero fcmge
	tworeg_zero fcmeq
	tworeg_zero fcmle
	tworeg_zero fcmlt

	.macro tworeg_misc, op
	\op	v0.2d, v1.2d
	\op	v0.2s, v1.2s
	\op	v0.4s, v1.4s
	\op	v0.4h, v1.4h
	\op	v0.8h, v1.8h
	.endm

	tworeg_misc fabs
	tworeg_misc fneg

	tworeg_misc frintn
	tworeg_misc frinta
	tworeg_misc frintp

	tworeg_misc frintm
	tworeg_misc frintx
	tworeg_misc frintz
	tworeg_misc frinti

	tworeg_misc fcvtns
	tworeg_misc fcvtnu
	tworeg_misc fcvtps
	tworeg_misc fcvtpu

	tworeg_misc fcvtms
	tworeg_misc fcvtmu
	tworeg_misc fcvtzs
	tworeg_misc fcvtzu

	tworeg_misc fcvtas
	tworeg_misc fcvtau

	tworeg_misc scvtf
	tworeg_misc ucvtf
	tworeg_misc frecpe
	tworeg_misc frsqrte
	tworeg_misc fsqrt

	/* Scalar two-register misc.  */

	.macro stworeg_zero, op
	\op	d0, d1, #0.0
	\op	s0, s1, #0.0
	\op	h0, h1, #0.0
	\op	h0, h0, #0.0
	.endm

	stworeg_zero fcmgt
	stworeg_zero fcmge
	stworeg_zero fcmeq
	stworeg_zero fcmle
	stworeg_zero fcmlt

	.macro stworeg_misc, op
	\op	d0, d1
	\op	s0, s1
	\op	h0, h1
	\op	h0, h0
	.endm

	stworeg_misc fcvtns
	stworeg_misc fcvtnu
	stworeg_misc fcvtps
	stworeg_misc fcvtpu

	stworeg_misc fcvtms
	stworeg_misc fcvtmu
	stworeg_misc fcvtzs
	stworeg_misc fcvtzu

	stworeg_misc fcvtas
	stworeg_misc fcvtau

	stworeg_misc scvtf
	stworeg_misc ucvtf

	stworeg_misc frecpe
	stworeg_misc frsqrte
	stworeg_misc frecpx

	/* Vector indexed element.  */

	.macro indexed_elem, op
	\op	v1.2d, v2.2d, v3.d[1]
	\op	v1.2s, v2.2s, v3.s[2]
	\op	v1.4s, v2.4s, v3.s[1]
	\op	v0.4h, v0.4h, v0.h[0]
	\op	v1.4h, v2.4h, v3.h[0]
	\op	v0.8h, v0.8h, v0.h[0]
	\op	v1.8h, v2.8h, v3.h[0]
	\op	v1.2d, v5.2d, v10.d[0]
	\op	v8.2s, v0.2s, v11.s[3]
	\op	v0.4h, v9.4h, v15.h[7]
	.endm

	indexed_elem fmla
	indexed_elem fmls

	indexed_elem fmul
	indexed_elem fmulx

	/* Scalar indexed element.  */

	.macro sindexed_elem, op
	\op	d1, d2, v3.d[1]
	\op	s1, s2, v3.s[1]
	\op	h1, h2, v3.h[1]
	\op	h0, h0, v0.h[0]
	.endm

	sindexed_elem fmla
	sindexed_elem fmls

	sindexed_elem fmul
	sindexed_elem fmulx

	/* Adv.SIMD across lanes.  */

	.macro across_lanes, op
	\op	s1, v2.4s
	\op	h1, v2.4h
	\op	h1, v2.8h
	\op	h0, v0.4h
	\op	h0, v0.8h
	.endm

	across_lanes fmaxnmv
	across_lanes fmaxv
	across_lanes fminnmv
	across_lanes fminv

	/* Adv.SIMD modified immediate.  */

	fmov	v1.2d, #2.0
	fmov	v1.2s, #2.0
	fmov	v1.4s, #2.0
	fmov	v1.4h, #2.0
	fmov	v1.8h, #2.0
	fmov	v0.4h, #1.0
	fmov	v0.8h, #1.0

	/* Adv.SIMD scalar pairwise.  */

	.macro scalar_pairwise, op
	\op	d1, v2.2d
	\op	s1, v2.2s
	\op	h1, v2.2h
	\op	h0, v0.2h
	.endm

	scalar_pairwise fmaxnmp
	scalar_pairwise faddp
	scalar_pairwise fmaxp
	scalar_pairwise fminnmp
	scalar_pairwise fminp

	/* Adv.SIMD shift by immediate.  */

	.macro shift_imm, op
	\op v1.2d, v2.2d, #3
	\op v1.2s, v2.2s, #3
	\op v1.4s, v2.4s, #3
	\op v1.4h, v2.4h, #3
	\op v1.8h, v2.8h, #3
	\op v0.4h, v0.4h, #1
	\op v0.8h, v0.8h, #1
	.endm

	shift_imm scvtf
	shift_imm fcvtzs
	shift_imm ucvtf
	shift_imm fcvtzu

	/* Adv.SIMD scalar shift by immediate.  */

	.macro sshift_imm, op
	\op d1, d2, #3
	\op s1, s2, #3
	\op h1, h2, #3
	\op h0, h0, #1
	.endm

	sshift_imm scvtf
	sshift_imm fcvtzs
	sshift_imm ucvtf
	sshift_imm fcvtzu
