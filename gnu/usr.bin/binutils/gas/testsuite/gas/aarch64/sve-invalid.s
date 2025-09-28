// Instructions in this file are invalid unless explicitly marked "OK".
// Other files provide more extensive testing of valid instructions;
// the only purpose of the valid instructions in this file is to show
// that the general form of the operands is correct.

	fmov	z1, z2
	fmov	z1, #1.0
	fmov	z1, #0.0

	not	z0.s,p1/
	not	z0.s,p1/,z2.s
	not	z0.s,p1/c,z2.s

	movprfx	z0.h, z1.h
	movprfx	z0, z1.h
	movprfx	z0.h, z1
	movprfx	z0.h, z1.s

	movprfx	z0, p1/m, z1
	movprfx	z0, p1/z, z1
	movprfx	z0.b, p1/m, z1
	movprfx	z0.b, p1/z, z1

	movprfx	z0, p1/m, z1.b
	movprfx	z0, p1/z, z1.b
	movprfx	z0.h, p1/m, z1.b
	movprfx	z0.h, p1/z, z1.b
	movprfx	z0.b, p1, z1.b

	movprfx	p0, p1

	ldr	p0.b, [x1]
	ldr	z0.b, [x1]

	str	p0.b, [x1]
	str	z0.b, [x1]

	mov	z0, b0
	mov	z0, z1
	mov	p0, p1

	add	z0, z0, z2
	add	z0, z0, #2
	add	z0, z1, z2
	add	z0, z1, #1
	add	z0.b, z1.b, #1
	add	z0.b, z0.h, #1

	mov	z0.b, z32.b
	mov	p0.b, p16.b

	cmpeq	p0.b, p8/z, z1.b, z2.b
	cmpeq	p0.b, p15/z, z1.b, z2.b

	ld1w	z0.s, p0, [x0]
	ld1w	z0.s, p0/m, [x0]
	cmpeq	p0.b, p0, z1.b, z2.b
	cmpeq	p0.b, p0/m, z1.b, z2.b
	add	z0.s, p0, z0.s, z1.s
	add	z0.s, p0/z, z0.s, z1.s
	st1w	z0.s, p0/z, [x0]
	st1w	z0.s, p0/m, [x0]

	ld1b	z0, p1/z, [x1]
	ld1h	z0, p1/z, [x1]
	ld1w	z0, p1/z, [x1]
	ld1d	z0, p1/z, [x1]

	ldff1b	z0, p1/z, [x1, xzr]
	ldff1h	z0, p1/z, [x1, xzr, lsl #1]
	ldff1w	z0, p1/z, [x1, xzr, lsl #2]
	ldff1d	z0, p1/z, [x1, xzr, lsl #3]

	ldnf1b	z0, p1/z, [x1]
	ldnf1h	z0, p1/z, [x1]
	ldnf1w	z0, p1/z, [x1]
	ldnf1d	z0, p1/z, [x1]

	ldnt1b	z0, p1/z, [x1]
	ldnt1h	z0, p1/z, [x1]
	ldnt1w	z0, p1/z, [x1]
	ldnt1d	z0, p1/z, [x1]

	st1b	z0, p1/z, [x1]
	st1h	z0, p1/z, [x1]
	st1w	z0, p1/z, [x1]
	st1d	z0, p1/z, [x1]

	stnt1b	z0, p1/z, [x1]
	stnt1h	z0, p1/z, [x1]
	stnt1w	z0, p1/z, [x1]
	stnt1d	z0, p1/z, [x1]

	ld1b	{z0}, p1/z, [x1]
	ld1h	{z0}, p1/z, [x1]
	ld1w	{z0}, p1/z, [x1]
	ld1d	{z0}, p1/z, [x1]

	ldff1b	{z0}, p1/z, [x1, xzr]
	ldff1h	{z0}, p1/z, [x1, xzr, lsl #1]
	ldff1w	{z0}, p1/z, [x1, xzr, lsl #2]
	ldff1d	{z0}, p1/z, [x1, xzr, lsl #3]

	ldnf1b	{z0}, p1/z, [x1]
	ldnf1h	{z0}, p1/z, [x1]
	ldnf1w	{z0}, p1/z, [x1]
	ldnf1d	{z0}, p1/z, [x1]

	ldnt1b	{z0}, p1/z, [x1]
	ldnt1h	{z0}, p1/z, [x1]
	ldnt1w	{z0}, p1/z, [x1]
	ldnt1d	{z0}, p1/z, [x1]

	st1b	{z0}, p1/z, [x1]
	st1h	{z0}, p1/z, [x1]
	st1w	{z0}, p1/z, [x1]
	st1d	{z0}, p1/z, [x1]

	stnt1b	{z0}, p1/z, [x1]
	stnt1h	{z0}, p1/z, [x1]
	stnt1w	{z0}, p1/z, [x1]
	stnt1d	{z0}, p1/z, [x1]

	ld1b	{x0}, p1/z, [x1]
	ld1b	{b0}, p1/z, [x1]
	ld1b	{h0}, p1/z, [x1]
	ld1b	{s0}, p1/z, [x1]
	ld1b	{d0}, p1/z, [x1]
	ld1b	{v0.2s}, p1/z, [x1]

	ld2b	{z0.b, z1}, p1/z, [x1]
	ld2b	{z0.b, z1.h}, p1/z, [x1]
	ld2b	{z0.b, z1.s}, p1/z, [x1]
	ld2b	{z0.b, z1.d}, p1/z, [x1]
	ld2b	{z0.h, z1}, p1/z, [x1]
	ld2b	{z0.h, z1.s}, p1/z, [x1]
	ld2b	{z0.h, z1.d}, p1/z, [x1]
	ld2b	{z0.s, z1}, p1/z, [x1]
	ld2b	{z0.s, z1.d}, p1/z, [x1]
	ld2b	{z0.d, z1}, p1/z, [x1]

	ld1b	z0.b, p1/z, [x1, #-9, mul vl]
	ld1b	z0.b, p1/z, [x1, #-8, mul vl]		// OK
	ld1b	z0.b, p1/z, [x1, #0, mul #1]
	ld1b	z0.b, p1/z, [x1, #0, mul vl #1]
	ld1b	z0.b, p1/z, [x1, #foo, mul vl]
	ld1b	z0.b, p1/z, [x1, #1]
	ld1b	z0.b, p1/z, [x1, #7, mul vl]		// OK
	ld1b	z0.b, p1/z, [x1, #7, mul vl]!
	ld1b	z0.b, p1/z, [x1, #8, mul vl]

	ld2b	{z0.b, z1.b}, p1/z, [x1, #-18, mul vl]
	ld2b	{z0.b, z1.b}, p1/z, [x1, #-17, mul vl]
	ld2b	{z0.b, z1.b}, p1/z, [x1, #-16, mul vl]	// OK
	ld2b	{z0.b, z1.b}, p1/z, [x1, #foo, mul vl]
	ld2b	{z0.b, z1.b}, p1/z, [x1, #1, mul vl]
	ld2b	{z0.b, z1.b}, p1/z, [x1, #14, mul vl]	// OK
	ld2b	{z0.b, z1.b}, p1/z, [x1, #14, mul vl]!
	ld2b	{z0.b, z1.b}, p1/z, [x1, #16, mul vl]

	ld3b	{z0.b-z2.b}, p1/z, [x1, #-27, mul vl]
	ld3b	{z0.b-z2.b}, p1/z, [x1, #-26, mul vl]
	ld3b	{z0.b-z2.b}, p1/z, [x1, #-25, mul vl]
	ld3b	{z0.b-z2.b}, p1/z, [x1, #-24, mul vl]	// OK
	ld3b	{z0.b-z2.b}, p1/z, [x1, #foo, mul vl]
	ld3b	{z0.b-z2.b}, p1/z, [x1, #1, mul vl]
	ld3b	{z0.b-z2.b}, p1/z, [x1, #2, mul vl]
	ld3b	{z0.b-z2.b}, p1/z, [x1, #21, mul vl]	// OK
	ld3b	{z0.b-z2.b}, p1/z, [x1, #21, mul vl]!
	ld3b	{z0.b-z2.b}, p1/z, [x1, #24, mul vl]

	ld4b	{z0.b-z3.b}, p1/z, [x1, #-36, mul vl]
	ld4b	{z0.b-z3.b}, p1/z, [x1, #-35, mul vl]
	ld4b	{z0.b-z3.b}, p1/z, [x1, #-34, mul vl]
	ld4b	{z0.b-z3.b}, p1/z, [x1, #-33, mul vl]
	ld4b	{z0.b-z3.b}, p1/z, [x1, #-32, mul vl]	// OK
	ld4b	{z0.b-z3.b}, p1/z, [x1, #foo, mul vl]
	ld4b	{z0.b-z3.b}, p1/z, [x1, #1, mul vl]
	ld4b	{z0.b-z3.b}, p1/z, [x1, #2, mul vl]
	ld4b	{z0.b-z3.b}, p1/z, [x1, #3, mul vl]
	ld4b	{z0.b-z3.b}, p1/z, [x1, #28, mul vl]	// OK
	ld4b	{z0.b-z3.b}, p1/z, [x1, #28, mul vl]!
	ld4b	{z0.b-z3.b}, p1/z, [x1, #32, mul vl]

	prfb	pldl1keep, p1, [x1, #-33, mul vl]
	prfb	pldl1keep, p1, [x1, #-32, mul vl]	// OK
	prfb	pldl1keep, p1, [x1, #foo, mul vl]
	prfb	pldl1keep, p1, [x1, #1]
	prfb	pldl1keep, p1, [x1, #31, mul vl]	// OK
	prfb	pldl1keep, p1, [x1, #31, mul vl]!
	prfb	pldl1keep, p1, [x1, #32, mul vl]

	ldr	z0, [x1, #-257, mul vl]
	ldr	z0, [x1, #-256, mul vl]			// OK
	ldr	z0, [x1, #foo, mul vl]
	ldr	z0, [x1, #1]
	ldr	z0, [x1, #255, mul vl]			// OK
	ldr	z0, [x1, #255, mul vl]!
	ldr	z0, [x1, #256, mul vl]

	ld1rb	z0.b, p1/z, [x1, #-1]
	ld1rb	z0.b, p1/z, [x1, #0]			// OK
	ld1rb	z0.b, p1/z, [x1, #foo]
	ld1rb	z0.b, p1/z, [x1, #1,mul vl]
	ld1rb	z0.b, p1/z, [x1, #63]			// OK
	ld1rb	z0.b, p1/z, [x1, #63]!
	ld1rb	z0.b, p1/z, [x1], #63
	ld1rb	z0.b, p1/z, [x1, #64]

	ld1rh	z0.h, p1/z, [x1, #-2]
	ld1rh	z0.h, p1/z, [x1, #-1]
	ld1rh	z0.h, p1/z, [x1, #0]			// OK
	ld1rh	z0.h, p1/z, [x1, #foo]
	ld1rh	z0.h, p1/z, [x1, #1]
	ld1rh	z0.h, p1/z, [x1, #2,mul vl]
	ld1rh	z0.h, p1/z, [x1, #126]			// OK
	ld1rh	z0.h, p1/z, [x1, #126]!
	ld1rh	z0.h, p1/z, [x1], #126
	ld1rh	z0.h, p1/z, [x1, #128]

	ld1rw	z0.s, p1/z, [x1, #-4]
	ld1rw	z0.s, p1/z, [x1, #-1]
	ld1rw	z0.s, p1/z, [x1, #0]			// OK
	ld1rw	z0.s, p1/z, [x1, #foo]
	ld1rw	z0.s, p1/z, [x1, #1]
	ld1rw	z0.s, p1/z, [x1, #2]
	ld1rw	z0.s, p1/z, [x1, #4,mul vl]
	ld1rw	z0.s, p1/z, [x1, #252]			// OK
	ld1rw	z0.s, p1/z, [x1, #252]!
	ld1rw	z0.s, p1/z, [x1], #252
	ld1rw	z0.s, p1/z, [x1, #256]

	ld1rd	z0.d, p1/z, [x1, #-8]
	ld1rd	z0.d, p1/z, [x1, #-1]
	ld1rd	z0.d, p1/z, [x1, #0]			// OK
	ld1rd	z0.d, p1/z, [x1, #foo]
	ld1rd	z0.d, p1/z, [x1, #1]
	ld1rd	z0.d, p1/z, [x1, #2]
	ld1rd	z0.d, p1/z, [x1, #4]
	ld1rd	z0.d, p1/z, [x1, #8,mul vl]
	ld1rd	z0.d, p1/z, [x1, #504]			// OK
	ld1rd	z0.d, p1/z, [x1, #504]!
	ld1rd	z0.d, p1/z, [x1], #504
	ld1rd	z0.d, p1/z, [x1, #512]

	ld1b	z0.b, p1/z, [x1,x2]			// OK
	ld1b	z0.b, p1/z, [x1,x2]!
	ld1b	z0.b, p1/z, [x1], x2
	ld1b	z0.b, p1/z, [x1,x2,lsl #1]
	ld1b	z0.b, p1/z, [x1,x2,lsl #2]
	ld1b	z0.b, p1/z, [x1,x2,lsl #3]
	ld1b	z0.b, p1/z, [x1,x2,lsl x3]
	ld1b	z0.b, p1/z, [x1,w2,sxtw]
	ld1b	z0.b, p1/z, [x1,w2,uxtw]

	ld1h	z0.h, p1/z, [x1,x2]
	ld1h	z0.h, p1/z, [x1,x2,lsl #1]		// OK
	ld1h	z0.h, p1/z, [x1,x2,lsl #1]!
	ld1h	z0.h, p1/z, [x1,x2,lsl #2]
	ld1h	z0.h, p1/z, [x1,x2,lsl #3]
	ld1h	z0.h, p1/z, [x1,x2,lsl x3]
	ld1h	z0.h, p1/z, [x1,w2,sxtw]
	ld1h	z0.h, p1/z, [x1,w2,uxtw]

	ld1w	z0.s, p1/z, [x1,x2]
	ld1w	z0.s, p1/z, [x1,x2,lsl #1]
	ld1w	z0.s, p1/z, [x1,x2,lsl #2]		// OK
	ld1w	z0.s, p1/z, [x1,x2,lsl #2]!
	ld1w	z0.s, p1/z, [x1,x2,lsl #3]
	ld1w	z0.s, p1/z, [x1,x2,lsl x3]
	ld1w	z0.s, p1/z, [x1,w2,sxtw]
	ld1w	z0.s, p1/z, [x1,w2,uxtw]

	ld1d	z0.d, p1/z, [x1,x2]
	ld1d	z0.d, p1/z, [x1,x2,lsl #1]
	ld1d	z0.d, p1/z, [x1,x2,lsl #2]
	ld1d	z0.d, p1/z, [x1,x2,lsl #3]		// OK
	ld1d	z0.d, p1/z, [x1,x2,lsl #3]!
	ld1d	z0.d, p1/z, [x1,x2,lsl x3]
	ld1d	z0.d, p1/z, [x1,w2,sxtw]
	ld1d	z0.d, p1/z, [x1,w2,uxtw]

	ld1b	z0.d, p1/z, [x1,z2.d]			// OK
	ld1b	z0.d, p1/z, [x1,z2.d,lsl #1]
	ld1b	z0.d, p1/z, [x1,z2.d,lsl #2]
	ld1b	z0.d, p1/z, [x1,z2.d,lsl #3]
	ld1b	z0.d, p1/z, [x1,z2.d,lsl x3]

	ld1h	z0.d, p1/z, [x1,z2.d]			// OK
	ld1h	z0.d, p1/z, [x1,z2.d,lsl #1]		// OK
	ld1h	z0.d, p1/z, [x1,z2.d,lsl #2]
	ld1h	z0.d, p1/z, [x1,z2.d,lsl #3]
	ld1h	z0.d, p1/z, [x1,z2.d,lsl x3]

	ld1w	z0.d, p1/z, [x1,z2.d]			// OK
	ld1w	z0.d, p1/z, [x1,z2.d,lsl #1]
	ld1w	z0.d, p1/z, [x1,z2.d,lsl #2]		// OK
	ld1w	z0.d, p1/z, [x1,z2.d,lsl #3]
	ld1w	z0.d, p1/z, [x1,z2.d,lsl x3]

	ld1d	z0.d, p1/z, [x1,z2.d]			// OK
	ld1d	z0.d, p1/z, [x1,z2.d,lsl #1]
	ld1d	z0.d, p1/z, [x1,z2.d,lsl #2]
	ld1d	z0.d, p1/z, [x1,z2.d,lsl #3]		// OK
	ld1d	z0.d, p1/z, [x1,z2.d,lsl x3]

	ld1b	z0.s, p1/z, [x1,z2.s,sxtw]		// OK
	ld1b	z0.s, p1/z, [x1,z2.s,sxtw #1]
	ld1b	z0.s, p1/z, [x1,z2.s,sxtw #2]
	ld1b	z0.s, p1/z, [x1,z2.s,sxtw #3]
	ld1b	z0.s, p1/z, [x1,z2.s,sxtw x3]

	ld1h	z0.s, p1/z, [x1,z2.s,sxtw]		// OK
	ld1h	z0.s, p1/z, [x1,z2.s,sxtw #1]		// OK
	ld1h	z0.s, p1/z, [x1,z2.s,sxtw #2]
	ld1h	z0.s, p1/z, [x1,z2.s,sxtw #3]
	ld1h	z0.s, p1/z, [x1,z2.s,sxtw x3]

	ld1w	z0.s, p1/z, [x1,z2.s,sxtw]		// OK
	ld1w	z0.s, p1/z, [x1,z2.s,sxtw #1]
	ld1w	z0.s, p1/z, [x1,z2.s,sxtw #2]		// OK
	ld1w	z0.s, p1/z, [x1,z2.s,sxtw #3]
	ld1w	z0.s, p1/z, [x1,z2.s,sxtw x3]

	ld1b	z0.s, p1/z, [x1,z2.s,uxtw]		// OK
	ld1b	z0.s, p1/z, [x1,z2.s,uxtw #1]
	ld1b	z0.s, p1/z, [x1,z2.s,uxtw #2]
	ld1b	z0.s, p1/z, [x1,z2.s,uxtw #3]
	ld1b	z0.s, p1/z, [x1,z2.s,uxtw x3]

	ld1h	z0.s, p1/z, [x1,z2.s,uxtw]		// OK
	ld1h	z0.s, p1/z, [x1,z2.s,uxtw #1]		// OK
	ld1h	z0.s, p1/z, [x1,z2.s,uxtw #2]
	ld1h	z0.s, p1/z, [x1,z2.s,uxtw #3]
	ld1h	z0.s, p1/z, [x1,z2.s,uxtw x3]

	ld1w	z0.s, p1/z, [x1,z2.s,uxtw]		// OK
	ld1w	z0.s, p1/z, [x1,z2.s,uxtw #1]
	ld1w	z0.s, p1/z, [x1,z2.s,uxtw #2]		// OK
	ld1w	z0.s, p1/z, [x1,z2.s,uxtw #3]
	ld1w	z0.s, p1/z, [x1,z2.s,uxtw x3]

	ld1b	z0.d, p1/z, [x1,z2.d,sxtw]		// OK
	ld1b	z0.d, p1/z, [x1,z2.d,sxtw #1]
	ld1b	z0.d, p1/z, [x1,z2.d,sxtw #2]
	ld1b	z0.d, p1/z, [x1,z2.d,sxtw #3]
	ld1b	z0.d, p1/z, [x1,z2.d,sxtw x3]

	ld1h	z0.d, p1/z, [x1,z2.d,sxtw]		// OK
	ld1h	z0.d, p1/z, [x1,z2.d,sxtw #1]		// OK
	ld1h	z0.d, p1/z, [x1,z2.d,sxtw #2]
	ld1h	z0.d, p1/z, [x1,z2.d,sxtw #3]
	ld1h	z0.d, p1/z, [x1,z2.d,sxtw x3]

	ld1w	z0.d, p1/z, [x1,z2.d,sxtw]		// OK
	ld1w	z0.d, p1/z, [x1,z2.d,sxtw #1]
	ld1w	z0.d, p1/z, [x1,z2.d,sxtw #2]		// OK
	ld1w	z0.d, p1/z, [x1,z2.d,sxtw #3]
	ld1w	z0.d, p1/z, [x1,z2.d,sxtw x3]

	ld1d	z0.d, p1/z, [x1,z2.d,sxtw]		// OK
	ld1d	z0.d, p1/z, [x1,z2.d,sxtw #1]
	ld1d	z0.d, p1/z, [x1,z2.d,sxtw #2]
	ld1d	z0.d, p1/z, [x1,z2.d,sxtw #3]		// OK
	ld1d	z0.d, p1/z, [x1,z2.d,sxtw x3]

	ld1b	z0.d, p1/z, [x1,z2.d,uxtw]		// OK
	ld1b	z0.d, p1/z, [x1,z2.d,uxtw #1]
	ld1b	z0.d, p1/z, [x1,z2.d,uxtw #2]
	ld1b	z0.d, p1/z, [x1,z2.d,uxtw #3]
	ld1b	z0.d, p1/z, [x1,z2.d,uxtw x3]

	ld1h	z0.d, p1/z, [x1,z2.d,uxtw]		// OK
	ld1h	z0.d, p1/z, [x1,z2.d,uxtw #1]		// OK
	ld1h	z0.d, p1/z, [x1,z2.d,uxtw #2]
	ld1h	z0.d, p1/z, [x1,z2.d,uxtw #3]
	ld1h	z0.d, p1/z, [x1,z2.d,uxtw x3]

	ld1w	z0.d, p1/z, [x1,z2.d,uxtw]		// OK
	ld1w	z0.d, p1/z, [x1,z2.d,uxtw #1]
	ld1w	z0.d, p1/z, [x1,z2.d,uxtw #2]		// OK
	ld1w	z0.d, p1/z, [x1,z2.d,uxtw #3]
	ld1w	z0.d, p1/z, [x1,z2.d,uxtw x3]

	ld1d	z0.d, p1/z, [x1,z2.d,uxtw]		// OK
	ld1d	z0.d, p1/z, [x1,z2.d,uxtw #1]
	ld1d	z0.d, p1/z, [x1,z2.d,uxtw #2]
	ld1d	z0.d, p1/z, [x1,z2.d,uxtw #3]		// OK
	ld1d	z0.d, p1/z, [x1,z2.d,uxtw x3]

	ld1b	z0.d, p1/z, [z2.d,#-1]
	ld1b	z0.d, p1/z, [z2.d,#0]			// OK
	ld1b	z0.d, p1/z, [z2.d,#foo]
	ld1b	z0.d, p1/z, [z2.d,#1,mul vl]
	ld1b	z0.d, p1/z, [z2.d,#31]			// OK
	ld1b	z0.d, p1/z, [z2.d,#32]

	ld1h	z0.d, p1/z, [z2.d,#-2]
	ld1h	z0.d, p1/z, [z2.d,#-1]
	ld1h	z0.d, p1/z, [z2.d,#0]			// OK
	ld1h	z0.d, p1/z, [z2.d,#foo]
	ld1h	z0.d, p1/z, [z2.d,#1]
	ld1h	z0.d, p1/z, [z2.d,#2,mul vl]
	ld1h	z0.d, p1/z, [z2.d,#62]			// OK
	ld1h	z0.d, p1/z, [z2.d,#64]

	ld1w	z0.d, p1/z, [z2.d,#-4]
	ld1w	z0.d, p1/z, [z2.d,#-1]
	ld1w	z0.d, p1/z, [z2.d,#0]			// OK
	ld1w	z0.d, p1/z, [z2.d,#foo]
	ld1w	z0.d, p1/z, [z2.d,#1]
	ld1w	z0.d, p1/z, [z2.d,#2]
	ld1w	z0.d, p1/z, [z2.d,#4,mul vl]
	ld1w	z0.d, p1/z, [z2.d,#124]			// OK
	ld1w	z0.d, p1/z, [z2.d,#128]

	ld1d	z0.d, p1/z, [z2.d,#-8]
	ld1d	z0.d, p1/z, [z2.d,#-1]
	ld1d	z0.d, p1/z, [z2.d,#0]			// OK
	ld1d	z0.d, p1/z, [z2.d,#foo]
	ld1d	z0.d, p1/z, [z2.d,#1]
	ld1d	z0.d, p1/z, [z2.d,#2]
	ld1d	z0.d, p1/z, [z2.d,#4]
	ld1d	z0.d, p1/z, [z2.d,#8,mul vl]
	ld1d	z0.d, p1/z, [z2.d,#248]			// OK
	ld1d	z0.d, p1/z, [z2.d,#256]

	adr	z0.s, [z1.s,z2.s,lsl #-1]
	adr	z0.s, [z1.s,z2.s]			// OK
	adr	z0.s, [z1.s,z2.s,lsl #1]		// OK
	adr	z0.s, [z1.s,z2.s,lsl #2]		// OK
	adr	z0.s, [z1.s,z2.s,lsl #3]		// OK
	adr	z0.s, [z1.s,z2.s,lsl #4]
	adr	z0.s, [z1.s,z2.s,lsl x3]
	adr	z0.s, [z1.s,z2.d]
	adr	z0.s, [z1.s,x2]
	adr	z0.s, [z1.d,z2.s]
	adr	z0.s, [z1.d,w2]
	adr	z0.s, [x1,z2.s]
	adr	z0.s, [x1,z2.d]
	adr	z0.s, [z1.d,x2]
	adr	z0.s, [x1,x2]

	adr	z0.d, [z1.d,z2.d,lsl #-1]
	adr	z0.d, [z1.d,z2.d]			// OK
	adr	z0.d, [z1.d,z2.d,lsl #1]		// OK
	adr	z0.d, [z1.d,z2.d,lsl #2]		// OK
	adr	z0.d, [z1.d,z2.d,lsl #3]		// OK
	adr	z0.d, [z1.d,z2.d,lsl #4]
	adr	z0.d, [z1.d,z2.d,lsl x3]

	adr	z0.s, [z1.s,z2.s,sxtw]

	adr	z0.d, [z1.d,z2.d,sxtw #-1]
	adr	z0.d, [z1.d,z2.d,sxtw]			// OK
	adr	z0.d, [z1.d,z2.d,sxtw #1]		// OK
	adr	z0.d, [z1.d,z2.d,sxtw #2]		// OK
	adr	z0.d, [z1.d,z2.d,sxtw #3]		// OK
	adr	z0.d, [z1.d,z2.d,sxtw #4]
	adr	z0.d, [z1.d,z2.d,sxtw x3]

	adr	z0.s, [z1.s,z2.s,uxtw]

	adr	z0.d, [z1.d,z2.d,uxtw #-1]
	adr	z0.d, [z1.d,z2.d,uxtw]			// OK
	adr	z0.d, [z1.d,z2.d,uxtw #1]		// OK
	adr	z0.d, [z1.d,z2.d,uxtw #2]		// OK
	adr	z0.d, [z1.d,z2.d,uxtw #3]		// OK
	adr	z0.d, [z1.d,z2.d,uxtw #4]
	adr	z0.d, [z1.d,z2.d,uxtw x3]

	ld1b	z0.b, p0/z, [x1,xzr]
	ld1b	z0.h, p0/z, [x1,xzr]
	ld1b	z0.s, p0/z, [x1,xzr]
	ld1b	z0.d, p0/z, [x1,xzr]
	ld1sb	z0.h, p0/z, [x1,xzr]
	ld1sb	z0.s, p0/z, [x1,xzr]
	ld1sb	z0.d, p0/z, [x1,xzr]

	ld1h	z0.h, p0/z, [x1,xzr,lsl #1]
	ld1h	z0.s, p0/z, [x1,xzr,lsl #1]
	ld1h	z0.d, p0/z, [x1,xzr,lsl #1]
	ld1sh	z0.s, p0/z, [x1,xzr,lsl #1]
	ld1sh	z0.d, p0/z, [x1,xzr,lsl #1]

	ld1w	z0.s, p0/z, [x1,xzr,lsl #2]
	ld1w	z0.d, p0/z, [x1,xzr,lsl #2]
	ld1sw	z0.d, p0/z, [x1,xzr,lsl #2]

	ld1d	z0.d, p0/z, [x1,xzr,lsl #3]

	ld2b	{z0.b-z1.b}, p0/z, [x1,xzr]
	ld2h	{z0.h-z1.h}, p0/z, [x1,xzr,lsl #1]
	ld2w	{z0.s-z1.s}, p0/z, [x1,xzr,lsl #2]
	ld2d	{z0.d-z1.d}, p0/z, [x1,xzr,lsl #3]

	ld3b	{z0.b-z2.b}, p0/z, [x1,xzr]
	ld3h	{z0.h-z2.h}, p0/z, [x1,xzr,lsl #1]
	ld3w	{z0.s-z2.s}, p0/z, [x1,xzr,lsl #2]
	ld3d	{z0.d-z2.d}, p0/z, [x1,xzr,lsl #3]

	ld4b	{z0.b-z3.b}, p0/z, [x1,xzr]
	ld4h	{z0.h-z3.h}, p0/z, [x1,xzr,lsl #1]
	ld4w	{z0.s-z3.s}, p0/z, [x1,xzr,lsl #2]
	ld4d	{z0.d-z3.d}, p0/z, [x1,xzr,lsl #3]

	ldff1b	z0.b, p0/z, [x1,xzr]		// OK
	ldff1b	z0.h, p0/z, [x1,xzr]		// OK
	ldff1b	z0.s, p0/z, [x1,xzr]		// OK
	ldff1b	z0.d, p0/z, [x1,xzr]		// OK
	ldff1sb	z0.h, p0/z, [x1,xzr]		// OK
	ldff1sb	z0.s, p0/z, [x1,xzr]		// OK
	ldff1sb	z0.d, p0/z, [x1,xzr]		// OK

	ldff1h	z0.h, p0/z, [x1,xzr,lsl #1]	// OK
	ldff1h	z0.s, p0/z, [x1,xzr,lsl #1]	// OK
	ldff1h	z0.d, p0/z, [x1,xzr,lsl #1]	// OK
	ldff1sh	z0.s, p0/z, [x1,xzr,lsl #1]	// OK
	ldff1sh	z0.d, p0/z, [x1,xzr,lsl #1]	// OK

	ldff1w	z0.s, p0/z, [x1,xzr,lsl #2]	// OK
	ldff1w	z0.d, p0/z, [x1,xzr,lsl #2]	// OK
	ldff1sw	z0.d, p0/z, [x1,xzr,lsl #2]	// OK

	ldff1d	z0.d, p0/z, [x1,xzr,lsl #3]	// OK

	ldnt1b	z0.b, p0/z, [x1,xzr]
	ldnt1h	z0.h, p0/z, [x1,xzr,lsl #1]
	ldnt1w	z0.s, p0/z, [x1,xzr,lsl #2]
	ldnt1d	z0.d, p0/z, [x1,xzr,lsl #3]

	st1b	z0.b, p0, [x1,xzr]
	st1b	z0.h, p0, [x1,xzr]
	st1b	z0.s, p0, [x1,xzr]
	st1b	z0.d, p0, [x1,xzr]

	st1h	z0.h, p0, [x1,xzr,lsl #1]
	st1h	z0.s, p0, [x1,xzr,lsl #1]
	st1h	z0.d, p0, [x1,xzr,lsl #1]

	st1w	z0.s, p0, [x1,xzr,lsl #2]
	st1w	z0.d, p0, [x1,xzr,lsl #2]

	st1d	z0.d, p0, [x1,xzr,lsl #3]

	st2b	{z0.b-z1.b}, p0, [x1,xzr]
	st2h	{z0.h-z1.h}, p0, [x1,xzr,lsl #1]
	st2w	{z0.s-z1.s}, p0, [x1,xzr,lsl #2]
	st2d	{z0.d-z1.d}, p0, [x1,xzr,lsl #3]

	st3b	{z0.b-z2.b}, p0, [x1,xzr]
	st3h	{z0.h-z2.h}, p0, [x1,xzr,lsl #1]
	st3w	{z0.s-z2.s}, p0, [x1,xzr,lsl #2]
	st3d	{z0.d-z2.d}, p0, [x1,xzr,lsl #3]

	st4b	{z0.b-z3.b}, p0, [x1,xzr]
	st4h	{z0.h-z3.h}, p0, [x1,xzr,lsl #1]
	st4w	{z0.s-z3.s}, p0, [x1,xzr,lsl #2]
	st4d	{z0.d-z3.d}, p0, [x1,xzr,lsl #3]

	stnt1b	z0.b, p0, [x1,xzr]
	stnt1h	z0.h, p0, [x1,xzr,lsl #1]
	stnt1w	z0.s, p0, [x1,xzr,lsl #2]
	stnt1d	z0.d, p0, [x1,xzr,lsl #3]

	prfb	pldl1keep, p0, [x1,xzr]
	prfh	pldl1keep, p0, [x1,xzr,lsl #1]
	prfw	pldl1keep, p0, [x1,xzr,lsl #2]
	prfd	pldl1keep, p0, [x1,xzr,lsl #3]

	add	z0.b, z0.b, #-257
	add	z0.b, z0.b, #-256			// OK
	add	z0.b, z0.b, #255			// OK
	add	z0.b, z0.b, #256
	add	z0.b, z0.b, #1, lsl #1
	add	z0.b, z0.b, #0, lsl #8
	add	z0.b, z0.b, #1, lsl #8

	add	z0.h, z0.h, #-65537
	add	z0.h, z0.h, #-65536 + 257
	add	z0.h, z0.h, #-32767
	add	z0.h, z0.h, #-32768 + 255
	add	z0.h, z0.h, #-257
	add	z0.h, z0.h, #-255
	add	z0.h, z0.h, #-129
	add	z0.h, z0.h, #-128
	add	z0.h, z0.h, #-127
	add	z0.h, z0.h, #-1
	add	z0.h, z0.h, #0				// OK
	add	z0.h, z0.h, #256			// OK
	add	z0.h, z0.h, #257
	add	z0.h, z0.h, #32768-255
	add	z0.h, z0.h, #32767
	add	z0.h, z0.h, #65536 - 255
	add	z0.h, z0.h, #65536 - 129
	add	z0.h, z0.h, #65536 - 128
	add	z0.h, z0.h, #65535
	add	z0.h, z0.h, #65536
	add	z0.h, z0.h, #1, lsl #1
	add	z0.h, z0.h, #-257, lsl #8
	add	z0.h, z0.h, #256, lsl #8

	add	z0.s, z0.s, #-256
	add	z0.s, z0.s, #-255
	add	z0.s, z0.s, #-129
	add	z0.s, z0.s, #-128
	add	z0.s, z0.s, #-1
	add	z0.s, z0.s, #0				// OK
	add	z0.s, z0.s, #256			// OK
	add	z0.s, z0.s, #257
	add	z0.s, z0.s, #32768-255
	add	z0.s, z0.s, #32767
	add	z0.s, z0.s, #65536
	add	z0.s, z0.s, #0x100000000
	add	z0.s, z0.s, #1, lsl #1
	add	z0.s, z0.s, #-1, lsl #8
	add	z0.s, z0.s, #256, lsl #8

	add	z0.d, z0.d, #-256
	add	z0.d, z0.d, #-255
	add	z0.d, z0.d, #-129
	add	z0.d, z0.d, #-128
	add	z0.d, z0.d, #-1
	add	z0.d, z0.d, #0				// OK
	add	z0.d, z0.d, #256			// OK
	add	z0.d, z0.d, #257
	add	z0.d, z0.d, #32768-255
	add	z0.d, z0.d, #32767
	add	z0.d, z0.d, #65536
	add	z0.d, z0.d, #0x100000000
	add	z0.d, z0.d, #1, lsl #1
	add	z0.d, z0.d, #-1, lsl #8
	add	z0.d, z0.d, #256, lsl #8

	dup	z0.b, #-257
	dup	z0.b, #-256				// OK
	dup	z0.b, #255				// OK
	dup	z0.b, #256
	dup	z0.b, #1, lsl #1
	dup	z0.b, #0, lsl #8
	dup	z0.b, #1, lsl #8

	dup	z0.h, #-65537
	dup	z0.h, #-32767
	dup	z0.h, #-32768 + 255
	dup	z0.h, #-257
	dup	z0.h, #-255
	dup	z0.h, #-129
	dup	z0.h, #-128				// OK
	dup	z0.h, #127				// OK
	dup	z0.h, #128
	dup	z0.h, #255
	dup	z0.h, #257
	dup	z0.h, #32768-255
	dup	z0.h, #32767
	dup	z0.h, #65536 - 255
	dup	z0.h, #65536 - 129
	dup	z0.h, #65536
	dup	z0.h, #1, lsl #1
	dup	z0.h, #-257, lsl #8
	dup	z0.h, #256, lsl #8

	dup	z0.s, #-65536
	dup	z0.s, #-32769
	dup	z0.s, #-32767
	dup	z0.s, #-32768 + 255
	dup	z0.s, #-257
	dup	z0.s, #-255
	dup	z0.s, #-129
	dup	z0.s, #-128				// OK
	dup	z0.s, #127				// OK
	dup	z0.s, #128
	dup	z0.s, #255
	dup	z0.s, #257
	dup	z0.s, #32768-255
	dup	z0.s, #32767
	dup	z0.s, #32768
	dup	z0.s, #65536
	dup	z0.s, #0xffffff7f
	dup	z0.s, #0x100000000
	dup	z0.s, #1, lsl #1
	dup	z0.s, #-129, lsl #8
	dup	z0.s, #128, lsl #8

	dup	z0.d, #-65536
	dup	z0.d, #-32769
	dup	z0.d, #-32767
	dup	z0.d, #-32768 + 255
	dup	z0.d, #-257
	dup	z0.d, #-255
	dup	z0.d, #-129
	dup	z0.d, #-128				// OK
	dup	z0.d, #127				// OK
	dup	z0.d, #128
	dup	z0.d, #255
	dup	z0.d, #257
	dup	z0.d, #32768-255
	dup	z0.d, #32767
	dup	z0.d, #32768
	dup	z0.d, #65536
	dup	z0.d, #0xffffff7f
	dup	z0.d, #0x100000000
	dup	z0.d, #1, lsl #1
	dup	z0.d, #-129, lsl #8
	dup	z0.d, #128, lsl #8

	and	z0.b, z0.b, #0x01			// OK
	and	z0.b, z0.b, #0x0101
	and	z0.b, z0.b, #0x01010101
	and	z0.b, z0.b, #0x0101010101010101
	and	z0.b, z0.b, #0x7f			// OK
	and	z0.b, z0.b, #0x7f7f
	and	z0.b, z0.b, #0x7f7f7f7f
	and	z0.b, z0.b, #0x7f7f7f7f7f7f7f7f
	and	z0.b, z0.b, #0x80			// OK
	and	z0.b, z0.b, #0x8080
	and	z0.b, z0.b, #0x80808080
	and	z0.b, z0.b, #0x8080808080808080
	and	z0.b, z0.b, #0xfe			// OK
	and	z0.b, z0.b, #0xfefe
	and	z0.b, z0.b, #0xfefefefe
	and	z0.b, z0.b, #0xfefefefefefefefe
	and	z0.b, z0.b, #0x00010001
	and	z0.b, z0.b, #0x0001000100010001
	and	z0.b, z0.b, #0x7fff
	and	z0.b, z0.b, #0x7fff7fff
	and	z0.b, z0.b, #0x7fff7fff7fff7fff
	and	z0.b, z0.b, #0x8000
	and	z0.b, z0.b, #0x80008000
	and	z0.b, z0.b, #0x8000800080008000
	and	z0.b, z0.b, #0xfffe
	and	z0.b, z0.b, #0xfffefffe
	and	z0.b, z0.b, #0xfffefffefffefffe
	and	z0.b, z0.b, #0x0000000100000001
	and	z0.b, z0.b, #0x7fffffff
	and	z0.b, z0.b, #0x7fffffff7fffffff
	and	z0.b, z0.b, #0x80000000
	and	z0.b, z0.b, #0x8000000080000000
	and	z0.b, z0.b, #0xfffffffe
	and	z0.b, z0.b, #0xfffffffefffffffe
	and	z0.b, z0.b, #0x7fffffffffffffff
	and	z0.b, z0.b, #0x8000000000000000
	and	z0.b, z0.b, #0xfffffffffffffffe		// OK

	and	z0.h, z0.h, #0x0101			// OK
	and	z0.h, z0.h, #0x01010101
	and	z0.h, z0.h, #0x0101010101010101
	and	z0.h, z0.h, #0x7f7f			// OK
	and	z0.h, z0.h, #0x7f7f7f7f
	and	z0.h, z0.h, #0x7f7f7f7f7f7f7f7f
	and	z0.h, z0.h, #0x8080			// OK
	and	z0.h, z0.h, #0x80808080
	and	z0.h, z0.h, #0x8080808080808080
	and	z0.h, z0.h, #0xfefe			// OK
	and	z0.h, z0.h, #0xfefefefe
	and	z0.h, z0.h, #0xfefefefefefefefe
	and	z0.h, z0.h, #0x00010001
	and	z0.h, z0.h, #0x0001000100010001
	and	z0.h, z0.h, #0x7fff			// OK
	and	z0.h, z0.h, #0x7fff7fff
	and	z0.h, z0.h, #0x7fff7fff7fff7fff
	and	z0.h, z0.h, #0x8000			// OK
	and	z0.h, z0.h, #0x80008000
	and	z0.h, z0.h, #0x8000800080008000
	and	z0.h, z0.h, #0xfffe			// OK
	and	z0.h, z0.h, #0xfffefffe
	and	z0.h, z0.h, #0xfffefffefffefffe
	and	z0.h, z0.h, #0x0000000100000001
	and	z0.h, z0.h, #0x7fffffff
	and	z0.h, z0.h, #0x7fffffff7fffffff
	and	z0.h, z0.h, #0x80000000
	and	z0.h, z0.h, #0x8000000080000000
	and	z0.h, z0.h, #0xfffffffe
	and	z0.h, z0.h, #0xfffffffefffffffe
	and	z0.h, z0.h, #0x7fffffffffffffff
	and	z0.h, z0.h, #0x8000000000000000

	and	z0.s, z0.s, #0x01010101			// OK
	and	z0.s, z0.s, #0x0101010101010101
	and	z0.s, z0.s, #0x7f7f7f7f			// OK
	and	z0.s, z0.s, #0x7f7f7f7f7f7f7f7f
	and	z0.s, z0.s, #0x80808080			// OK
	and	z0.s, z0.s, #0x8080808080808080
	and	z0.s, z0.s, #0xfefefefe			// OK
	and	z0.s, z0.s, #0xfefefefefefefefe
	and	z0.s, z0.s, #0x00010001			// OK
	and	z0.s, z0.s, #0x0001000100010001
	and	z0.s, z0.s, #0x7fff7fff			// OK
	and	z0.s, z0.s, #0x7fff7fff7fff7fff
	and	z0.s, z0.s, #0x80008000			// OK
	and	z0.s, z0.s, #0x8000800080008000
	and	z0.s, z0.s, #0xfffefffe			// OK
	and	z0.s, z0.s, #0xfffefffefffefffe
	and	z0.s, z0.s, #0x0000000100000001
	and	z0.s, z0.s, #0x7fffffff			// OK
	and	z0.s, z0.s, #0x7fffffff7fffffff
	and	z0.s, z0.s, #0x80000000			// OK
	and	z0.s, z0.s, #0x8000000080000000
	and	z0.s, z0.s, #0xfffffffe			// OK
	and	z0.s, z0.s, #0xfffffffefffffffe
	and	z0.s, z0.s, #0x7fffffffffffffff
	and	z0.s, z0.s, #0x8000000000000000

	and	z0.d, z0.d, #0xc			// OK
	and	z0.d, z0.d, #0xd
	and	z0.d, z0.d, #0xe			// OK

	bic	z0.b, z0.b, #0x01			// OK
	bic	z0.b, z0.b, #0x0101
	bic	z0.b, z0.b, #0x01010101
	bic	z0.b, z0.b, #0x0101010101010101
	bic	z0.b, z0.b, #0x7f			// OK
	bic	z0.b, z0.b, #0x7f7f
	bic	z0.b, z0.b, #0x7f7f7f7f
	bic	z0.b, z0.b, #0x7f7f7f7f7f7f7f7f
	bic	z0.b, z0.b, #0x80			// OK
	bic	z0.b, z0.b, #0x8080
	bic	z0.b, z0.b, #0x80808080
	bic	z0.b, z0.b, #0x8080808080808080
	bic	z0.b, z0.b, #0xfe			// OK
	bic	z0.b, z0.b, #0xfefe
	bic	z0.b, z0.b, #0xfefefefe
	bic	z0.b, z0.b, #0xfefefefefefefefe
	bic	z0.b, z0.b, #0x00010001
	bic	z0.b, z0.b, #0x0001000100010001
	bic	z0.b, z0.b, #0x7fff
	bic	z0.b, z0.b, #0x7fff7fff
	bic	z0.b, z0.b, #0x7fff7fff7fff7fff
	bic	z0.b, z0.b, #0x8000
	bic	z0.b, z0.b, #0x80008000
	bic	z0.b, z0.b, #0x8000800080008000
	bic	z0.b, z0.b, #0xfffe
	bic	z0.b, z0.b, #0xfffefffe
	bic	z0.b, z0.b, #0xfffefffefffefffe
	bic	z0.b, z0.b, #0x0000000100000001
	bic	z0.b, z0.b, #0x7fffffff
	bic	z0.b, z0.b, #0x7fffffff7fffffff
	bic	z0.b, z0.b, #0x80000000
	bic	z0.b, z0.b, #0x8000000080000000
	bic	z0.b, z0.b, #0xfffffffe
	bic	z0.b, z0.b, #0xfffffffefffffffe
	bic	z0.b, z0.b, #0x7fffffffffffffff
	bic	z0.b, z0.b, #0x8000000000000000
	bic	z0.b, z0.b, #0xfffffffffffffffe		// OK

	bic	z0.h, z0.h, #0x0101			// OK
	bic	z0.h, z0.h, #0x01010101
	bic	z0.h, z0.h, #0x0101010101010101
	bic	z0.h, z0.h, #0x7f7f			// OK
	bic	z0.h, z0.h, #0x7f7f7f7f
	bic	z0.h, z0.h, #0x7f7f7f7f7f7f7f7f
	bic	z0.h, z0.h, #0x8080			// OK
	bic	z0.h, z0.h, #0x80808080
	bic	z0.h, z0.h, #0x8080808080808080
	bic	z0.h, z0.h, #0xfefe			// OK
	bic	z0.h, z0.h, #0xfefefefe
	bic	z0.h, z0.h, #0xfefefefefefefefe
	bic	z0.h, z0.h, #0x00010001
	bic	z0.h, z0.h, #0x0001000100010001
	bic	z0.h, z0.h, #0x7fff			// OK
	bic	z0.h, z0.h, #0x7fff7fff
	bic	z0.h, z0.h, #0x7fff7fff7fff7fff
	bic	z0.h, z0.h, #0x8000			// OK
	bic	z0.h, z0.h, #0x80008000
	bic	z0.h, z0.h, #0x8000800080008000
	bic	z0.h, z0.h, #0xfffe			// OK
	bic	z0.h, z0.h, #0xfffefffe
	bic	z0.h, z0.h, #0xfffefffefffefffe
	bic	z0.h, z0.h, #0x0000000100000001
	bic	z0.h, z0.h, #0x7fffffff
	bic	z0.h, z0.h, #0x7fffffff7fffffff
	bic	z0.h, z0.h, #0x80000000
	bic	z0.h, z0.h, #0x8000000080000000
	bic	z0.h, z0.h, #0xfffffffe
	bic	z0.h, z0.h, #0xfffffffefffffffe
	bic	z0.h, z0.h, #0x7fffffffffffffff
	bic	z0.h, z0.h, #0x8000000000000000

	bic	z0.s, z0.s, #0x01010101			// OK
	bic	z0.s, z0.s, #0x0101010101010101
	bic	z0.s, z0.s, #0x7f7f7f7f			// OK
	bic	z0.s, z0.s, #0x7f7f7f7f7f7f7f7f
	bic	z0.s, z0.s, #0x80808080			// OK
	bic	z0.s, z0.s, #0x8080808080808080
	bic	z0.s, z0.s, #0xfefefefe			// OK
	bic	z0.s, z0.s, #0xfefefefefefefefe
	bic	z0.s, z0.s, #0x00010001			// OK
	bic	z0.s, z0.s, #0x0001000100010001
	bic	z0.s, z0.s, #0x7fff7fff			// OK
	bic	z0.s, z0.s, #0x7fff7fff7fff7fff
	bic	z0.s, z0.s, #0x80008000			// OK
	bic	z0.s, z0.s, #0x8000800080008000
	bic	z0.s, z0.s, #0xfffefffe			// OK
	bic	z0.s, z0.s, #0xfffefffefffefffe
	bic	z0.s, z0.s, #0x0000000100000001
	bic	z0.s, z0.s, #0x7fffffff			// OK
	bic	z0.s, z0.s, #0x7fffffff7fffffff
	bic	z0.s, z0.s, #0x80000000			// OK
	bic	z0.s, z0.s, #0x8000000080000000
	bic	z0.s, z0.s, #0xfffffffe			// OK
	bic	z0.s, z0.s, #0xfffffffefffffffe
	bic	z0.s, z0.s, #0x7fffffffffffffff
	bic	z0.s, z0.s, #0x8000000000000000

	bic	z0.d, z0.d, #0xc			// OK
	bic	z0.d, z0.d, #0xd
	bic	z0.d, z0.d, #0xe			// OK

	fcmeq	p0.s, p1/z, z2.s, #0			// OK
	fcmeq	p0.s, p1/z, z2.s, #0.0			// OK
	fcmeq	p0.s, p1/z, z2.s, #1
	fcmeq	p0.s, p1/z, z2.s, #1.0

	fadd	z0.s, p1/m, z0.s, #0
	fadd	z0.s, p1/m, z0.s, #0.0
	fadd	z0.s, p1/m, z0.s, #0.5			// OK
	fadd	z0.s, p1/m, z0.s, #1			// OK
	fadd	z0.s, p1/m, z0.s, #1.0			// OK
	fadd	z0.s, p1/m, z0.s, #1.5
	fadd	z0.s, p1/m, z0.s, #2
	fadd	z0.s, p1/m, z0.s, #2.0

	fmul	z0.s, p1/m, z0.s, #0
	fmul	z0.s, p1/m, z0.s, #0.0
	fmul	z0.s, p1/m, z0.s, #0.5			// OK
	fmul	z0.s, p1/m, z0.s, #1			// OK
	fmul	z0.s, p1/m, z0.s, #1.0
	fmul	z0.s, p1/m, z0.s, #1.5
	fmul	z0.s, p1/m, z0.s, #2			// OK
	fmul	z0.s, p1/m, z0.s, #2.0			// OK

	fmax	z0.s, p1/m, z0.s, #0			// OK
	fmax	z0.s, p1/m, z0.s, #0.0			// OK
	fmax	z0.s, p1/m, z0.s, #0.5
	fmax	z0.s, p1/m, z0.s, #1			// OK
	fmax	z0.s, p1/m, z0.s, #1.0			// OK
	fmax	z0.s, p1/m, z0.s, #1.5
	fmax	z0.s, p1/m, z0.s, #2
	fmax	z0.s, p1/m, z0.s, #2.0

	ptrue	p1.b, vl0
	ptrue	p1.b, vl255
	ptrue	p1.b, #-1
	ptrue	p1.b, #0				// OK
	ptrue	p1.b, #31				// OK
	ptrue	p1.b, #32
	ptrue	p1.b, x0
	ptrue	p1.b, z0.s

	cntb	x0, vl0
	cntb	x0, vl255
	cntb	x0, #-1
	cntb	x0, #0					// OK
	cntb	x0, #31					// OK
	cntb	x0, #32
	cntb	x0, x0
	cntb	x0, z0.s
	cntb	x0, mul #1
	cntb	x0, pow2, mul #0
	cntb	x0, pow2, mul #1			// OK
	cntb	x0, pow2, mul #16			// OK
	cntb	x0, pow2, mul #17
	cntb	x0, pow2, #1

	prfb	pldl0keep, p1, [x0]
	prfb	pldl1keep, p1, [x0]			// OK
	prfb	pldl2keep, p1, [x0]			// OK
	prfb	pldl3keep, p1, [x0]			// OK
	prfb	pldl4keep, p1, [x0]
	prfb	#-1, p1, [x0]
	prfb	#0, p1, [x0]				// OK
	prfb	#15, p1, [x0]				// OK
	prfb	#16, p1, [x0]
	prfb	x0, p1, [x0]
	prfb	z0.s, p1, [x0]

	lsl	z0.b, z0.b, #-1
	lsl	z0.b, z0.b, #0				// OK
	lsl	z0.b, z0.b, #1				// OK
	lsl	z0.b, z0.b, #7				// OK
	lsl	z0.b, z0.b, #8
	lsl	z0.b, z0.b, #9
	lsl	z0.b, z0.b, x0

	lsl	z0.h, z0.h, #-1
	lsl	z0.h, z0.h, #0				// OK
	lsl	z0.h, z0.h, #1				// OK
	lsl	z0.h, z0.h, #15				// OK
	lsl	z0.h, z0.h, #16
	lsl	z0.h, z0.h, #17

	lsl	z0.s, z0.s, #-1
	lsl	z0.s, z0.s, #0				// OK
	lsl	z0.s, z0.s, #1				// OK
	lsl	z0.s, z0.s, #31				// OK
	lsl	z0.s, z0.s, #32
	lsl	z0.s, z0.s, #33

	lsl	z0.d, z0.d, #-1
	lsl	z0.d, z0.d, #0				// OK
	lsl	z0.d, z0.d, #1				// OK
	lsl	z0.d, z0.d, #63				// OK
	lsl	z0.d, z0.d, #64
	lsl	z0.d, z0.d, #65

	lsl	z0.b, p1/m, z0.b, #-1
	lsl	z0.b, p1/m, z0.b, #0			// OK
	lsl	z0.b, p1/m, z0.b, #1			// OK
	lsl	z0.b, p1/m, z0.b, #7			// OK
	lsl	z0.b, p1/m, z0.b, #8
	lsl	z0.b, p1/m, z0.b, #9
	lsl	z0.b, p1/m, z0.b, x0

	lsl	z0.h, p1/m, z0.h, #-1
	lsl	z0.h, p1/m, z0.h, #0			// OK
	lsl	z0.h, p1/m, z0.h, #1			// OK
	lsl	z0.h, p1/m, z0.h, #15			// OK
	lsl	z0.h, p1/m, z0.h, #16
	lsl	z0.h, p1/m, z0.h, #17

	lsl	z0.s, p1/m, z0.s, #-1
	lsl	z0.s, p1/m, z0.s, #0			// OK
	lsl	z0.s, p1/m, z0.s, #1			// OK
	lsl	z0.s, p1/m, z0.s, #31			// OK
	lsl	z0.s, p1/m, z0.s, #32
	lsl	z0.s, p1/m, z0.s, #33

	lsl	z0.d, p1/m, z0.d, #-1
	lsl	z0.d, p1/m, z0.d, #0			// OK
	lsl	z0.d, p1/m, z0.d, #1			// OK
	lsl	z0.d, p1/m, z0.d, #63			// OK
	lsl	z0.d, p1/m, z0.d, #64
	lsl	z0.d, p1/m, z0.d, #65

	lsr	z0.b, z0.b, #-1
	lsr	z0.b, z0.b, #0
	lsr	z0.b, z0.b, #1				// OK
	lsr	z0.b, z0.b, #7				// OK
	lsr	z0.b, z0.b, #8				// OK
	lsr	z0.b, z0.b, #9
	lsr	z0.b, z0.b, x0

	lsr	z0.h, z0.h, #-1
	lsr	z0.h, z0.h, #0
	lsr	z0.h, z0.h, #1				// OK
	lsr	z0.h, z0.h, #15				// OK
	lsr	z0.h, z0.h, #16				// OK
	lsr	z0.h, z0.h, #17

	lsr	z0.s, z0.s, #-1
	lsr	z0.s, z0.s, #0
	lsr	z0.s, z0.s, #1				// OK
	lsr	z0.s, z0.s, #31				// OK
	lsr	z0.s, z0.s, #32				// OK
	lsr	z0.s, z0.s, #33

	lsr	z0.d, z0.d, #-1
	lsr	z0.d, z0.d, #0
	lsr	z0.d, z0.d, #1				// OK
	lsr	z0.d, z0.d, #63				// OK
	lsr	z0.d, z0.d, #64				// OK
	lsr	z0.d, z0.d, #65

	lsr	z0.b, p1/m, z0.b, #-1
	lsr	z0.b, p1/m, z0.b, #0
	lsr	z0.b, p1/m, z0.b, #1			// OK
	lsr	z0.b, p1/m, z0.b, #7			// OK
	lsr	z0.b, p1/m, z0.b, #8			// OK
	lsr	z0.b, p1/m, z0.b, #9
	lsr	z0.b, p1/m, z0.b, x0

	lsr	z0.h, p1/m, z0.h, #-1
	lsr	z0.h, p1/m, z0.h, #0
	lsr	z0.h, p1/m, z0.h, #1			// OK
	lsr	z0.h, p1/m, z0.h, #15			// OK
	lsr	z0.h, p1/m, z0.h, #16			// OK
	lsr	z0.h, p1/m, z0.h, #17

	lsr	z0.s, p1/m, z0.s, #-1
	lsr	z0.s, p1/m, z0.s, #0
	lsr	z0.s, p1/m, z0.s, #1			// OK
	lsr	z0.s, p1/m, z0.s, #31			// OK
	lsr	z0.s, p1/m, z0.s, #32			// OK
	lsr	z0.s, p1/m, z0.s, #33

	lsr	z0.d, p1/m, z0.d, #-1
	lsr	z0.d, p1/m, z0.d, #0
	lsr	z0.d, p1/m, z0.d, #1			// OK
	lsr	z0.d, p1/m, z0.d, #63			// OK
	lsr	z0.d, p1/m, z0.d, #64			// OK
	lsr	z0.d, p1/m, z0.d, #65

	index	z0.s, #-17, #1
	index	z0.s, #-16, #1				// OK
	index	z0.s, #15, #1				// OK
	index	z0.s, #16, #1

	index	z0.s, #0, #-17
	index	z0.s, #0, #-16				// OK
	index	z0.s, #0, #15				// OK
	index	z0.s, #0, #16

	addpl	x0, sp, #-33
	addpl	x0, sp, #-32				// OK
	addpl	sp, x0, #31				// OK
	addpl	sp, x0, #32
	addpl	x0, xzr, #1
	addpl	xzr, x0, #1

	mul	z0.b, z0.b, #-129
	mul	z0.b, z0.b, #-128			// OK
	mul	z0.b, z0.b, #127			// OK
	mul	z0.b, z0.b, #128

	mul	z0.s, z0.s, #-129
	mul	z0.s, z0.s, #-128			// OK
	mul	z0.s, z0.s, #127			// OK
	mul	z0.s, z0.s, #128

	ftmad	z0.s, z0.s, z1.s, #-1
	ftmad	z0.s, z0.s, z1.s, #0			// OK
	ftmad	z0.s, z0.s, z1.s, #7			// OK
	ftmad	z0.s, z0.s, z1.s, #8
	ftmad	z0.s, z0.s, z1.s, z2.s

	cmphi	p0.s,p1/z,z2.s,#-1
	cmphi	p0.s,p1/z,z2.s,#0			// OK
	cmphi	p0.s,p1/z,z2.s,#127			// OK
	cmphi	p0.s,p1/z,z2.s,#128

	umax	z0.s, z0.s, #-1
	umax	z0.s, z0.s, #0				// OK
	umax	z0.s, z0.s, #255			// OK
	umax	z0.s, z0.s, #256

	ext	z0.b, z0.b, z1.b, #-1
	ext	z0.b, z0.b, z1.b, #0			// OK
	ext	z0.b, z0.b, z1.b, #255			// OK
	ext	z0.b, z0.b, z1.b, #256

	dup	z0.b, z1.b[-1]
	dup	z0.b, z1.b[0]				// OK
	dup	z0.b, z1.b[63]				// OK
	dup	z0.b, z1.b[64]
	dup	z0.b, z1.b[x0]
	dup	z0.b, z1[0]

	dup	z0.h, z1.h[-1]
	dup	z0.h, z1.h[0]				// OK
	dup	z0.h, z1.h[31]				// OK
	dup	z0.h, z1.h[32]
	dup	z0.h, z1.h[x0]

	dup	z0.s, z1.s[-1]
	dup	z0.s, z1.s[0]				// OK
	dup	z0.s, z1.s[15]				// OK
	dup	z0.s, z1.s[16]
	dup	z0.s, z1.s[x0]

	dup	z0.d, z1.d[-1]
	dup	z0.d, z1.d[0]				// OK
	dup	z0.d, z1.d[7]				// OK
	dup	z0.d, z1.d[8]
	dup	z0.d, z1.d[x0]

	fabd	z0.b, p0/m, z0.b, z0.b
	fabd	z0.q, p0/m, z0.q, z0.q

	fcadd	z0.b, p0/m, z0.b, z0.b, #90
	fcadd	z0.h, p0/m, z0.h, z0.h, #-180
	fcadd	z0.h, p0/m, z0.h, z0.h, #-90
	fcadd	z0.h, p0/m, z0.h, z0.h, #0
	fcadd	z0.h, p0/m, z0.h, z0.h, #89
	fcadd	z0.h, p0/m, z0.h, z0.h, #90.0
	fcadd	z0.h, p0/m, z0.h, z0.h, #180
	fcadd	z0.h, p0/m, z0.h, z0.h, #360
	fcadd	z0.h, p0/m, z0.h, z0.h, #450
	fcadd	z0.h, p0/z, z0.h, z0.h, #90
	fcadd	z0.h, p0/m, z1.h, z0.h, #90
	fcadd	z0.q, p0/m, z0.q, z0.q, #90

	fcmla	z0.b, p0/m, z0.b, z0.b, #90
	fcmla	z0.h, p0/m, z0.h, z0.h, #-180
	fcmla	z0.h, p0/m, z0.h, z0.h, #-90
	fcmla	z0.h, p0/m, z0.h, z0.h, #89
	fcmla	z0.h, p0/m, z0.h, z0.h, #90.0
	fcmla	z0.h, p0/m, z0.h, z0.h, #360
	fcmla	z0.h, p0/m, z0.h, z0.h, #450
	fcmla	z0.h, p0/z, z0.h, z0.h, #90
	fcmla	z0.q, p0/m, z0.q, z0.q, #90

	fcmla	z0.b, z1.b, z2.b[0], #0
	fcmla	z0.h, z1.h, z2.h[-1], #0
	fcmla	z0.h, z1.h, z2.h[4], #0
	fcmla	z0.h, z1.h, z8.h[0], #0
	fcmla	z0.h, z1.h, z2.h[0], #-180
	fcmla	z0.h, z1.h, z2.h[0], #-90
	fcmla	z0.h, z1.h, z2.h[0], #89
	fcmla	z0.h, z1.h, z2.h[0], #90.0
	fcmla	z0.h, z1.h, z2.h[0], #360
	fcmla	z0.h, z1.h, z2.h[0], #450
	fcmla	z0.s, z1.s, z2.s[-1], #0
	fcmla	z0.s, z1.s, z2.s[2], #0
	fcmla	z0.s, z1.s, z16.s[0], #0
	fcmla	z0.s, z1.s, z2.s[0], #-180
	fcmla	z0.s, z1.s, z2.s[0], #-90
	fcmla	z0.s, z1.s, z2.s[0], #89
	fcmla	z0.s, z1.s, z2.s[0], #90.0
	fcmla	z0.s, z1.s, z2.s[0], #360
	fcmla	z0.s, z1.s, z2.s[0], #450
	fcmla	z0.q, z1.q, z2.q[0], #0

	fmla	z0.b, z1.b, z2.b[0]
	fmla	z0.h, z1.h, z2.h[-1]
	fmla	z0.h, z1.h, z2.h[8]
	fmla	z0.h, z1.h, z8.h[0]
	fmla	z0.s, z1.s, z2.s[-1]
	fmla	z0.s, z1.s, z2.s[4]
	fmla	z0.s, z1.s, z8.s[0]
	fmla	z0.d, z1.d, z2.d[-1]
	fmla	z0.d, z1.d, z2.d[2]
	fmla	z0.d, z1.d, z16.d[0]
	fmla	z0.q, z1.q, z2.q[0]

	fmls	z0.b, z1.b, z2.b[0]
	fmls	z0.h, z1.h, z2.h[-1]
	fmls	z0.h, z1.h, z2.h[8]
	fmls	z0.h, z1.h, z8.h[0]
	fmls	z0.s, z1.s, z2.s[-1]
	fmls	z0.s, z1.s, z2.s[4]
	fmls	z0.s, z1.s, z8.s[0]
	fmls	z0.d, z1.d, z2.d[-1]
	fmls	z0.d, z1.d, z2.d[2]
	fmls	z0.d, z1.d, z16.d[0]
	fmls	z0.q, z1.q, z2.q[0]

	fmul	z0.b, z1.b, z2.b[0]
	fmul	z0.h, z1.h, z2.h[-1]
	fmul	z0.h, z1.h, z2.h[8]
	fmul	z0.h, z1.h, z8.h[0]
	fmul	z0.s, z1.s, z2.s[-1]
	fmul	z0.s, z1.s, z2.s[4]
	fmul	z0.s, z1.s, z8.s[0]
	fmul	z0.d, z1.d, z2.d[-1]
	fmul	z0.d, z1.d, z2.d[2]
	fmul	z0.d, z1.d, z16.d[0]
	fmul	z0.q, z1.q, z2.q[0]

	ld1rqb	{z0.b}, p0, [x0, #0]
	ld1rqb	{z0.b}, p0/m, [x0, #0]
	ld1rqb	{z0.b}, p8/z, [x0, #0]
	ld1rqb	{z0.b}, p0/z, [x0, #-144]
	ld1rqb	{z0.b}, p0/z, [x0, #-15]
	ld1rqb	{z0.b}, p0/z, [x0, #-14]
	ld1rqb	{z0.b}, p0/z, [x0, #-13]
	ld1rqb	{z0.b}, p0/z, [x0, #-12]
	ld1rqb	{z0.b}, p0/z, [x0, #-11]
	ld1rqb	{z0.b}, p0/z, [x0, #-10]
	ld1rqb	{z0.b}, p0/z, [x0, #-9]
	ld1rqb	{z0.b}, p0/z, [x0, #-8]
	ld1rqb	{z0.b}, p0/z, [x0, #-7]
	ld1rqb	{z0.b}, p0/z, [x0, #-6]
	ld1rqb	{z0.b}, p0/z, [x0, #-5]
	ld1rqb	{z0.b}, p0/z, [x0, #-4]
	ld1rqb	{z0.b}, p0/z, [x0, #-3]
	ld1rqb	{z0.b}, p0/z, [x0, #-2]
	ld1rqb	{z0.b}, p0/z, [x0, #-1]
	ld1rqb	{z0.b}, p0/z, [x0, #1]
	ld1rqb	{z0.b}, p0/z, [x0, #2]
	ld1rqb	{z0.b}, p0/z, [x0, #3]
	ld1rqb	{z0.b}, p0/z, [x0, #4]
	ld1rqb	{z0.b}, p0/z, [x0, #5]
	ld1rqb	{z0.b}, p0/z, [x0, #6]
	ld1rqb	{z0.b}, p0/z, [x0, #7]
	ld1rqb	{z0.b}, p0/z, [x0, #8]
	ld1rqb	{z0.b}, p0/z, [x0, #9]
	ld1rqb	{z0.b}, p0/z, [x0, #10]
	ld1rqb	{z0.b}, p0/z, [x0, #11]
	ld1rqb	{z0.b}, p0/z, [x0, #12]
	ld1rqb	{z0.b}, p0/z, [x0, #13]
	ld1rqb	{z0.b}, p0/z, [x0, #14]
	ld1rqb	{z0.b}, p0/z, [x0, #15]
	ld1rqb	{z0.b}, p0/z, [x0, #128]
	ld1rqb	{z0.h}, p0/z, [x0, #0]
	ld1rqb	{z0.s}, p0/z, [x0, #0]
	ld1rqb	{z0.d}, p0/z, [x0, #0]
	ld1rqb	{z0.q}, p0/z, [x0, #0]

	ld1rqb	{z0.b}, p0/z, [x0, xzr]
	ld1rqb	{z0.b}, p0/z, [x0, x1, lsl #1]
	ld1rqb	{z0.b}, p0/z, [x0, x1, lsl #2]
	ld1rqb	{z0.b}, p0/z, [x0, x1, lsl #3]

	ld1rqh	{z0.h}, p0/z, [x0, xzr, lsl #1]
	ld1rqh	{z0.h}, p0/z, [x0, x1]
	ld1rqh	{z0.h}, p0/z, [x0, x1, lsl #2]
	ld1rqh	{z0.h}, p0/z, [x0, x1, lsl #3]

	ld1rqw	{z0.s}, p0/z, [x0, xzr, lsl #2]
	ld1rqw	{z0.s}, p0/z, [x0, x1]
	ld1rqw	{z0.s}, p0/z, [x0, x1, lsl #1]
	ld1rqw	{z0.s}, p0/z, [x0, x1, lsl #3]

	ld1rqd	{z0.d}, p0/z, [x0, xzr, lsl #3]
	ld1rqd	{z0.d}, p0/z, [x0, x1]
	ld1rqd	{z0.d}, p0/z, [x0, x1, lsl #1]
	ld1rqd	{z0.d}, p0/z, [x0, x1, lsl #2]

	sdot	z0.b, z1.b, z2.b
	sdot	z0.h, z1.h, z2.h
	sdot	z0.s, z1.s, z2.s
	sdot	z0.d, z1.d, z2.d

	sdot	z0.b, z1.b, z2.b[0]
	sdot	z0.h, z1.h, z2.h[0]
	sdot	z0.s, z1.s, z2.s[0]
	sdot	z0.d, z1.d, z2.d[0]

	udot	z0.b, z1.b, z2.b
	udot	z0.h, z1.h, z2.h
	udot	z0.s, z1.s, z2.s
	udot	z0.d, z1.d, z2.d

	udot	z0.b, z1.b, z2.b[0]
	udot	z0.h, z1.h, z2.h[0]
	udot	z0.s, z1.s, z2.s[0]
	udot	z0.d, z1.d, z2.d[0]

	ld2b	{}, p0/z, [x0]
	ld2b	{.b}, p0/z, [x0]
	ld2b	{z0.b-}, p0/z, [x0]
	ld2b	{z0.b,}, p0/z, [x0]
	ld2b	{z0.b-z32.b}, p0/z, [x0]
	ld2b	{z0.b-v1.16b}, p0/z, [x0]
