// diagnostic.s Test file for diagnostic quality.

.text
	fmul,	s0, s1, s2
	fmul	,	s0, s1, s2
	fmul	, s0, s1, s2
	b.random	label1
	fmull   s0
	aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
	sys	1,c1,c3,3,
	ext	v0.8b, v1.8b, v2.8b, 8
	ext	v0.16b, v1.16b, v2.16b, 20
	svc	-1
	svc	65536
	ccmp	w0, 32, 10, le
	ccmp	x0, -1, 10, le
	tlbi	alle3is, x0
	tlbi	vaale1is
	tlbi	vaale1is x0
	msr	spsel, 3
	fcvtzu	x15, d31, #66
	scvtf	s0, w0, 33
	scvtf	s0, w0, 0
	smlal	v0.4s, v31.4h, v16.h[1]
	smlal	v0.4s, v31.4h, v15.h[8]
	add	sp, x0, x7, lsr #2
	add	x0, x0, x7, uxtx #5
	add	x0, xzr, x7, ror #5
	add	w0, wzr, w7, asr #32
	st2	{v0.4s, v1.4s}, [sp], #24
	ldr	q0, [x0, w0, uxtw #5]
	st2	{v0.4s, v1.4s, v2.4s, v3.4s}, [sp], #64
	adds	x1, sp, 2134, lsl #4
	movz	w0, 2134, lsl #8
	movz	w0, 2134, lsl #32
	movz	x0, 2134, lsl #47
	sbfiz	w0, w7, 15, 18
	sbfiz	w0, w7, 15, 0
	shll	v1.4s, v2.4h, #15
	shll	v1.4s, v2.4h, #32
	shl	v1.2s, v2.2s, 32
	sqshrn2	v2.16b, v3.8h, #17
	movi	v1.4h, 256
	movi	v1.4h, -129
	movi	v1.4h, 255, msl #8
	movi	d0, 256
	movi	v1.4h, 255, lsl #7
	movi	v1.4h, 255, lsl #16
	movi	v2.2s, 255, msl #0
	movi	v2.2s, 255, msl #15
	fmov	v1.2s, 1.01
	fmov	v1.2d, 1.01
	fmov	s3, 1.01
	fmov	d3, 1.01
	fcmp	d0, #1.0
	fcmp	d0, x0
	cmgt	v0.4s, v2.4s, #1
	fmov	d3, 1.00, lsl #3
	st2	{v0.4s, v1.4s}, [sp], sp
	st2	{v0.4s, v1.4s}, [sp], zr
	ldr	q0, [x0, w0, lsr #4]
	adds	x1, sp, 2134, uxtw #12
	movz	x0, 2134, lsl #64
	adds	sp, sp, 2134, lsl #12
	ldxrb	w2, [x0, #1]
	ldrb	w0, x1, x2, sxtx
	prfm	PLDL3KEEP, [x9, x15, sxtx #2]
	sysl	x7, #1, C16, C30, #1
	sysl	x7, #1, C15, C77, #1
	sysl	x7, #1, x15, C1, #1
	add	x0, xzr, x7, uxtx #5
	mov	x0, ##5
	bad expression
	mockup-op
	orr	x0. x0, #0xff, lsl #1
	movk	x1, #:abs_g1_s:s12
	movz	x1, #:abs_g1_s:s12, lsl #16
	prfm	pldl3strm, [sp, w0, sxtw #3]!
	prfm	0x2f, LABEL1
	dmb	#16
	tbz	w0, #40, 0x17c
	st2	{v4.2d, v5.2d, v6.2d}, [x3]
	ld2	{v1.4h, v0.4h}, [x1]
	isb	osh
	st2	{v4.2d, v5.2d, v6.2d}, \[x3\]
	ldnp	w7, w15, [x3, #3]
	stnp	x7, x15, [x3, #32]!
	ldnp	w7, w15, [x3, #256]
	movi	v1.2d, 4294967295, lsl #0
	movi	v1.8b, 97, lsl #8
	msr	dummy, x1
	fmov	s0, 0x42000000
	ldp	x0, x1, [x2, #4]
	ldp	x0, x1, [x2, #4]!
	ldp	x0, x1, [x2], #4
	stp	w0, w1, [x2, #3]
	stp	w0, w1, [x2, #2]!
	stp	w0, w1, [x2], #1
	cinc	w0, w1, al
	cinc	w0, w1, nv
	cset	w0, al
	cset	w0, nv

	# test diagnostic info on optional operand

	ret	kk
	clrex	x0
	clrex	w0
	clrex	kk
	sys	#0, c0, c0, #0, kk
	sys	#0, c0, c0, #0,

	casp w0,w1,w2,w3,[x4]

	# test warning of unpredictable load pairs
	ldp     x0, x0, [sp]
	ldp     d0, d0, [sp]
	ldp     x0, x0, [sp], #16
	ldnp    x0, x0, [sp]

	# test warning of unpredictable writeback
	ldr	x0, [x0, #8]!
	str	x0, [x0, #8]!
	str	x1, [x1], #8
	stp	x0, x1, [x0, #16]!
	ldp	x0, x1, [x1], #16
	adr	x2, :got:s1
	ldr	x0, [x0, :got:s1]

	# Test error of 32-bit base reg
	ldr	x1, [wsp, #8]!
	ldp	x6, x29, [w7, #8]!
	str	x30, [w11, #8]!
	stp	x8, x27, [wsp, #8]!

	# Test various valid load/store reg combination.
	# especially we shouldn't warn on xzr, although
	# xzr is with the same encoding 31 as sp.
	.macro ldst_pair_wb_2 op, reg1, reg2
	.irp base x3, x6, x25, sp
	\op	\reg1, \reg2, [\base], #16
	\op	\reg1, \reg2, [\base, #32]!
	\op	\reg2, \reg1, [\base], #32
	\op	\reg2, \reg1, [\base, #16]!
	.endr
	.endm

	.macro ldst_pair_wb_1 op, reg1, width
	.irp reg2 0, 14, 21, 23, 29
	ldst_pair_wb_2	\op, \reg1, \width\reg2
	.endr
	.endm

	.macro ldst_pair_wb_64 op
	.irp	reg1 x2, x15, x16, x27, x30, xzr
	ldst_pair_wb_1	\op, \reg1, x
	.endr
	.endm

	.macro ldst_pair_wb_32 op
	.irp	reg1 w1, w12, w16, w19, w30, wzr
	ldst_pair_wb_1	\op, \reg1, w
	.endr
	.endm

	.macro ldst_single_wb_1 op, reg
	.irp	base x1, x4, x13, x26, sp
	\op	\reg, [\base], #16
	.endr
	.endm

	.macro ldst_single_wb_32 op
	.irp reg w0, w3, w12, w21, w28, w30, wzr
	ldst_single_wb_1	\op, \reg
	.endr
	.endm

	.macro ldst_single_wb_64 op
	.irp reg x2, x5, x17, x23, x24, x30, xzr
	ldst_single_wb_1	\op, \reg
	.endr
	.endm

	ldst_pair_wb_32	stp
	ldst_pair_wb_64 stp

	ldst_pair_wb_32	ldp
	ldst_pair_wb_64 ldp

	ldst_pair_wb_64	ldpsw

	ldst_single_wb_32 str
	ldst_single_wb_64 str

	ldst_single_wb_32 strb

	ldst_single_wb_32 strh

	ldst_single_wb_32 ldr
	ldst_single_wb_64 ldr

	ldst_single_wb_32 ldrb

	ldst_single_wb_32 ldrh

	ldst_single_wb_32 ldrsb
	ldst_single_wb_64 ldrsb

	ldst_single_wb_32 ldrsh
	ldst_single_wb_64 ldrsh

	ldst_single_wb_64 ldrsw

	dup	v0.2d, v1.2d[-1]
	dup	v0.2d, v1.2d[0]
	dup	v0.2d, v1.2d[1]
	dup	v0.2d, v1.2d[2]
	dup	v0.2d, v1.2d[64]

	dup	v0.4s, v1.4s[-1]
	dup	v0.4s, v1.4s[0]
	dup	v0.4s, v1.4s[3]
	dup	v0.4s, v1.4s[4]
	dup	v0.4s, v1.4s[65]

	dup	v0.8h, v1.8h[-1]
	dup	v0.8h, v1.8h[0]
	dup	v0.8h, v1.8h[7]
	dup	v0.8h, v1.8h[8]
	dup	v0.8h, v1.8h[66]

	dup	v0.16b, v1.16b[-1]
	dup	v0.16b, v1.16b[0]
	dup	v0.16b, v1.16b[15]
	dup	v0.16b, v1.16b[16]
	dup	v0.16b, v1.16b[67]

	ld2	{v0.d, v1.d}[-1], [x0]
	ld2	{v0.d, v1.d}[0], [x0]
	ld2	{v0.d, v1.d}[1], [x0]
	ld2	{v0.d, v1.d}[2], [x0]
	ld2	{v0.d, v1.d}[64], [x0]

	ld2	{v0.s, v1.s}[-1], [x0]
	ld2	{v0.s, v1.s}[0], [x0]
	ld2	{v0.s, v1.s}[3], [x0]
	ld2	{v0.s, v1.s}[4], [x0]
	ld2	{v0.s, v1.s}[65], [x0]

	ld2	{v0.h, v1.h}[-1], [x0]
	ld2	{v0.h, v1.h}[0], [x0]
	ld2	{v0.h, v1.h}[7], [x0]
	ld2	{v0.h, v1.h}[8], [x0]
	ld2	{v0.h, v1.h}[66], [x0]

	ld2	{v0.b, v1.b}[-1], [x0]
	ld2	{v0.b, v1.b}[0], [x0]
	ld2	{v0.b, v1.b}[15], [x0]
	ld2	{v0.b, v1.b}[16], [x0]
	ld2	{v0.b, v1.b}[67], [x0]






	st2	{v0.4s, v1.4s}, [sp], xzr
	str	x1, [x2, sp]

	ldr	x0, [x1, #:lo12:foo] // OK
	ldnp	x1, x2, [x3, #:lo12:foo]
	ld1	{v0.4s}, [x3, #:lo12:foo]
	stuminl x0, [x3, #:lo12:foo]
	prfum	pldl1keep, [x3, #:lo12:foo]

	ldr	x0, [x3], x4
	ldnp	x1, x2, [x3], x4
	ld1	{v0.4s}, [x3], x4 // OK
	stuminl x0, [x3], x4
	prfum	pldl1keep, [x3], x4

	ldr	x0, [x1, #1, mul vl]
	ldr	x0, [x1, x2, mul vl]
	ldr	x0, [x1, x2, mul #1]
	ldr	x0, [x1, x2, mul #4]

	strb	w7, [x30, x0, mul]
	strb	w7, [x30, x0, mul #1]
	strb	w7, [x30, w0, mul]
	strb	w7, [x30, w0, mul #2]

	adds	x1, sp, 1, mul #1
	adds	x1, sp, 2, mul #255
	adds	x1, sp, 3, mul #256
	orr	x0, x0, #0xff, mul #1
	orr	x0, x0, #0xfe, mul #255
	orr	x0, x0, #0xfc, mul #256

	ip0 	.req 	x0
	ip1	.req 	x1
	lr	.req 	x2
	fp	.req 	x3

	stlxrb 	w26, w26, [x0]
	stlxrh	w26, w26, [x1]
	stlxr 	w26, w26, [x2]
	stlxrb 	w26, w27, [x26]
	stlxrh	w26, w27, [x26]
	stlxr 	w26, w27, [x26]
	stlxr 	w26, x27, [x26]
	stlxr	w26, x26, [x3]
	ldxp	x26, x26, [x5]
	ldxp	x26, x1, [x26]
	st4	{v0.16b-v3.16b}[4], [x0]
	stlxp	w3, w26, w26, [x3]

	ldr	x0, [1]
	ldr	x0, []
	ldr	x0, [,xzr]

	zip2	x1
	uxtw	d2
	usra	x3
	ushr	z4
	umull	z5
	umin	d6
	stur	v7
	sel	v8
	orn	d9
	frecpx	v10
	bics	z11
	rev	wsp
	orr	b12
	neg	p13
	fcvtpu	za14h
	fcmlt	z15
	clastb	sp
	ldr	sp
