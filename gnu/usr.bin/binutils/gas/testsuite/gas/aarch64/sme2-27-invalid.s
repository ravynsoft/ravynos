	.equ	x0, 1
	.equ	z0.s, 2
	.equ	p0, 3
	.equ	pn0, 4

	sqrshr	0, { z0.s - z1.s }, #1
	sqrshr	z0.h, 0, #1

	sqrshr	z0.h, { z1.s - z2.s }, #1
	sqrshr	z0.h, { z0.s - z1.s }, #0
	sqrshr	z0.h, { z0.s - z1.s }, #17
	sqrshr	z0.s, { z0.d - z1.d }, #1
	sqrshr	z0.h, { z0.s - z1.s }, x0
	sqrshr	z0.h, { z0.s - z1.s }, z0.s
	sqrshr	z0.h, { z0.s - z1.s }, p0
	sqrshr	z0.h, { z0.s - z1.s }, pn0

	sqrshr	z0.b, { z1.s - z4.s }, #1
	sqrshr	z0.b, { z2.s - z5.s }, #1
	sqrshr	z0.b, { z3.s - z6.s }, #1
	sqrshr	z0.b, { z0.s - z3.s }, #-1
	sqrshr	z0.b, { z0.s - z3.s }, #0
	sqrshr	z0.b, { z0.s - z3.s }, #33
	sqrshr	z0.b, { z0.d - z3.d }, #1
	sqrshr	z0.b, { z0.d - z3.d }, #65 // Double error
