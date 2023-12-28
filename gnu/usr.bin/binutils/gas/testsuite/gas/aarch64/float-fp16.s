/* Test file for AArch64 half-precision floating-point instructions.  */

	.text
	fccmp s0, s0, #0, eq
	fccmp h0, h0, #0, eq
	fccmp s1, s2, #0, le
	fccmp h1, h2, #0, le

	fccmpe s0, s0, #0, eq
	fccmpe h0, h0, #0, eq
	fccmpe s1, s2, #0, le
	fccmpe h1, h2, #0, le

	fcmp s0, s0
	fcmp h0, h0
	fcmp s1, s2
	fcmp h1, h2

	fcmpe s0, s0
	fcmpe h0, h0
	fcmpe s1, s2
	fcmpe h1, h2

	fcmp s0, #0.0
	fcmp h0, #0.0

	fcmpe s0, #0.0
	fcmpe h0, #0.0

	fcsel s0, s0, s1, eq
	fcsel h0, h0, h1, eq

	fmov x0, h0
	fmov w0, h0
	fmov h1, x0
	fmov h1, w0

	/* Scalar data-processing with one source.  */
	.macro sdp1src op
	\op     h0, h1
	\op     s0, s1
	\op     d0, d1
	.endm

	.text
	.irp op, fabs, fneg, fsqrt, frintn, frintp, frintm, frintz
	sdp1src \op
	.endr

	.irp op, frinta, frintx, frinti
	sdp1src \op
	.endr

	/* Scalar data-processing with two sources.  */
	.macro sdp2src op
	\op     h0, h1, h2
	\op     s0, s1, s2
	\op     d0, d1, d2
	.endm

	.text
	.irp op, fmul, fdiv, fadd, fsub, fmax, fmin, fmaxnm, fminnm, fnmul
	sdp2src \op
	.endr

	/* Scalar data-processing with three sources.  */
	.macro sdp3src op
	\op     h0, h1, h2, h3
	\op     s0, s1, s2, s3
	\op     d0, d1, d2, d3
	.endm

	.text
	.irp op, fmadd, fmsub, fnmadd, fnmsub
	sdp3src \op
	.endr

	/* Scalar conversion.  */

	.macro scvt_fix2fp op
	\op     s0, w1, #2
	\op     s0, x1, #3
	\op     h0, w1, #2
	\op     h0, x1, #3
	.endm

	.macro scvt_fp2fix op
	\op     w1, d0, #2
	\op     x1, d0, #3
	\op     w1, h0, #2
	\op     x1, h0, #3
	.endm

	.text

	fmov s0, #1.0
	fmov h0, #1.0

	.irp op, scvtf, ucvtf
	scvt_fix2fp \op
	.endr

	.irp op, fcvtzs, fcvtzu
	scvt_fp2fix \op
	.endr

	.macro scvt_fp2int op
	\op w1, s0
	\op x1, d0
	\op w1, h0
	\op x1, h0
	.endm

	.macro scvt_int2fp op
	\op s0, w1
	\op d0, x1
	\op h0, w1
	\op h0, x1
	.endm

	.text
	.irp op, fcvtns, fcvtnu, fcvtau, fcvtas
	scvt_fp2int \op
	.endr

	.text
	.irp op, fcvtps, fcvtpu, fcvtms, fcvtmu
	scvt_fp2int \op
	.endr

	.irp op, scvtf, ucvtf
	scvt_int2fp \op
	.endr

	/* FMOV.  */

	fmov d0, d1
	fmov s0, s1
	fmov h0, h1

	fmov x0, h1
	fmov w0, h1

	fmov h1, x0
	fmov h1, w0

	fmov w0, s1
	fmov x0, d1

	fmov s1, w0
	fmov d1, x0
