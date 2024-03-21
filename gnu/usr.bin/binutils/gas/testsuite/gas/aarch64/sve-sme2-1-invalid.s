	pfalse	pn0.h
	pfalse	pn0.s
	pfalse	pn0.d
	pfalse	pn0.q
	pfalse	pn0

	mov	pn0.b, p0.b
	mov	p0.b, pn0.b
	mov	pn0.b, pn1.h
	mov	pn0.h, pn1.b
	mov	pn0.h, pn1.h
	mov	pn0.s, pn1.s
	mov	pn0.d, pn1.d
	mov	pn0.q, pn1.q
	mov	pn0, pn1

	ldr	pn0.b, [x0]
	ldr	pn0.b, [xzr]
	ldr	pn0, [x0, #-257, mul vl]
	ldr	pn0, [x0, #256, mul vl]

	str	pn0.b, [x0]
	str	pn0.b, [xzr]
	str	pn0, [x0, #-257, mul vl]
	str	pn0, [x0, #256, mul vl]
