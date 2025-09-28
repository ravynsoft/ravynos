	.equ	x0, 0
	.equ	s0, 0
	.equ	z0, 0
	.equ	z0.s, 0
	.equ	p0, 0
	.equ	p0.b, 1

	cmeq	v0.4s, v1.4s, x0	// Error (wrong register type)
	cmeq	v0.4s, v1.4s, #x0	// OK
	cmeq	v0.4s, v1.4s, s0	// Error (wrong register type)
	cmeq	v0.4s, v1.4s, #s0	// OK
	cmeq	v0.4s, v1.4s, z0	// OK (for compatibility)
	cmeq	v0.4s, v1.4s, #z0	// OK
	cmeq	v0.4s, v1.4s, z0.s	// OK (for compatibility)
	cmeq	v0.4s, v1.4s, #z0.s	// OK
	cmeq	v0.4s, v1.4s, p0	// OK (for compatibility)
	cmeq	v0.4s, v1.4s, #p0	// OK
	cmeq	v0.4s, v1.4s, p0.b	// Error (not 0)
	cmeq	v0.4s, v1.4s, #p0.b	// Error (not 0)

	ldr	x1, [x0, x0]		// OK
	ldr	x1, [x0, #x0]		// OK
	ldr	x1, [x2, s0]		// OK (not considered a register here)
	ldr	x1, [x2, #s0]		// OK
	ldr	x1, [x2, z0]		// OK (for compatibility)
	ldr	x1, [x2, #z0]		// OK
	ldr	x2, [x2, z0.s]		// OK (for compatibility)
	ldr	x1, [x2, #z0.s]		// OK
	ldr	x2, [x2, p0]		// OK (not considered a register here)
	ldr	x1, [x2, #p0]		// OK
	ldr	x2, [x2, p0.b]		// OK (not considered a register here)
	ldr	x1, [x2, #p0.b]		// OK

	ldr	x1, [x0]		// OK
	ldr	x1, [s0]		// Error (not a base register)
	ldr	x1, [z0]		// Error
	ldr	x1, [z0.s]		// Error
	ldr	x1, [p0]		// Error (not a base register)
	ldr	x1, [p0.b]		// Error (not a base register)

	ldr	x0, [x1, x2, lsl x0]	// OK (not considered a register here)
	ldr	x0, [x1, x2, lsl #x0]	// OK
	ldr	x0, [x1, x2, lsl s0]	// OK (not considered a register here)
	ldr	x0, [x1, x2, lsl #s0]	// OK
	ldr	x0, [x1, x2, lsl z0]	// OK (not considered a register here)
	ldr	x0, [x1, x2, lsl #z0]	// OK
	ldr	x0, [x1, x2, lsl z0.s]	// OK (not considered a register here)
	ldr	x0, [x1, x2, lsl #z0.s]	// OK
	ldr	x0, [x1, x2, lsl p0]	// OK (not considered a register here)
	ldr	x0, [x1, x2, lsl #p0]	// OK
	ldr	x0, [x1, x2, lsl p0.b]	// Error (invalid shift amount)
	ldr	x0, [x1, x2, lsl #p0.b]	// Error (invalid shift amount)

	mov	x0, x0			// OK
	mov	x0, #x0			// OK
	mov	x0, s0			// OK (not considered a register here)
	mov	x0, #s0			// OK
	mov	x0, z0			// OK (not considered a register here)
	mov	x0, #z0			// OK
	mov	x0, z0.s		// OK (not considered a register here)
	mov	x0, #z0.s		// OK
	mov	x0, p0			// OK (not considered a register here)
	mov	x0, #p0			// OK
	mov	x0, p0.b		// OK (not considered a register here)
	mov	x0, #p0.b		// OK

	movk	x0, x0			// OK (not considered a register here)
	movk	x0, #x0			// OK
	movk	x0, s0			// OK (not considered a register here)
	movk	x0, #s0			// OK
	movk	x0, z0			// OK (not considered a register here)
	movk	x0, #z0			// OK
	movk	x0, z0.s		// OK (not considered a register here)
	movk	x0, #z0.s		// OK
	movk	x0, p0			// OK (not considered a register here)
	movk	x0, #p0			// OK
	movk	x0, p0.b		// OK (not considered a register here)
	movk	x0, #p0.b		// OK

	add	x0, x0, x0		// OK
	add	x0, x0, #x0		// OK
	add	x0, x0, s0		// OK (not considered a register here)
	add	x0, x0, #s0		// OK
	add	x0, x0, z0		// OK (not considered a register here)
	add	x0, x0, #z0		// OK
	add	x0, x0, z0.s		// OK (not considered a register here)
	add	x0, x0, #z0.s		// OK
	add	x0, x0, p0		// OK (not considered a register here)
	add	x0, x0, #p0		// OK
	add	x0, x0, p0.b		// OK (not considered a register here)
	add	x0, x0, #p0.b		// OK

	and	x0, x0, x0		// OK
	and	x0, x0, #x0		// Error (immediate out of range)
	and	x0, x0, s0		// Error (immediate out of range)
	and	x0, x0, #s0		// Error (immediate out of range)
	and	x0, x0, z0		// Error (immediate out of range)
	and	x0, x0, #z0		// Error (immediate out of range)
	and	x0, x0, z0.s		// Error (immediate out of range)
	and	x0, x0, #z0.s		// Error (immediate out of range)
	and	x0, x0, p0		// Error (immediate out of range)
	and	x0, x0, #p0		// Error (immediate out of range)
	and	x0, x0, p0.b		// OK (not considered a register here)
	and	x0, x0, #p0.b		// OK

	lsl	x0, x0, x0		// OK
	lsl	x0, x0, #x0		// OK
	lsl	x0, x0, s0		// Error (wrong register type)
	lsl	x0, x0, #s0		// OK
	lsl	x0, x0, z0		// OK (for compatibility)
	lsl	x0, x0, #z0		// OK
	lsl	x0, x0, z0.s		// OK (for compatibility)
	lsl	x0, x0, #z0.s		// OK
	lsl	x0, x0, p0		// OK (for compatibility)
	lsl	x0, x0, #p0		// OK
	lsl	x0, x0, p0.b		// OK (for compatibility)
	lsl	x0, x0, #p0.b		// OK

	adr	x0, x0			// OK (not considered a register here)
	adr	x0, #x0			// OK
	adr	x0, s0			// OK (not considered a register here)
	adr	x0, #s0			// OK
	adr	x0, z0			// OK (not considered a register here)
	adr	x0, #z0			// OK
	adr	x0, z0.s		// OK (not considered a register here)
	adr	x0, #z0.s		// OK
	adr	x0, p0			// OK (not considered a register here)
	adr	x0, #p0			// OK
	adr	x0, p0.b		// OK (not considered a register here)
	adr	x0, #p0.b		// OK

	svc	x0			// Error (immediate operand required)
	svc	#x0			// OK
	svc	s0			// Error (immediate operand required)
	svc	#s0			// OK
	svc	z0			// OK (for compatibility)
	svc	#z0			// OK
	svc	z0.s			// OK (for compatibility)
	svc	#z0.s			// OK
	svc	p0			// OK (for compatibility)
	svc	#p0			// OK
	svc	p0.b			// OK (for compatibility)
	svc	#p0.b			// OK
