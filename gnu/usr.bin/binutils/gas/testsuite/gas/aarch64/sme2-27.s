	.equ	x0, 1
	.equ	z0.s, 2
	.equ	p0, 3
	.equ	pn0, 4

	sqrshr	z0.h, { z0.s - z1.s }, #1
	sqrshr	z31.h, { z0.s - z1.s }, #1
	sqrshr	z0.h, { z30.s - z31.s }, #1
	sqrshr	z0.h, { z0.s - z1.s }, #16
	sqrshr	z14.h, { z22.s - z23.s }, #7

	sqrshr	z0.h, { z0.s - z1.s }, #x0
	sqrshr	z0.h, { z0.s - z1.s }, #z0.s
	sqrshr	z0.h, { z0.s - z1.s }, #p0
	sqrshr	z0.h, { z0.s - z1.s }, #pn0

	sqrshr	z0.b, { z0.s - z3.s }, #1
	sqrshr	z31.b, { z0.s - z3.s }, #1
	sqrshr	z0.b, { z28.s - z31.s }, #1
	sqrshr	z0.b, { z0.s - z3.s }, #32
	sqrshr	z6.b, { z12.s - z15.s }, #25

	sqrshr	z0.h, { z0.d - z3.d }, #1
	sqrshr	z31.h, { z0.d - z3.d }, #1
	sqrshr	z0.h, { z28.d - z31.d }, #1
	sqrshr	z0.h, { z0.d - z3.d }, #64
	sqrshr	z25.h, { z20.d - z23.d }, #50

	// Invalid SQRSHR
	.inst 0xc13fd800
	.inst 0xc120d800

	sqrshru	z0.h, { z0.s - z1.s }, #1
	sqrshru	z31.h, { z0.s - z1.s }, #1
	sqrshru	z0.h, { z30.s - z31.s }, #1
	sqrshru	z0.h, { z0.s - z1.s }, #16
	sqrshru	z14.h, { z22.s - z23.s }, #7

	sqrshru	z0.b, { z0.s - z3.s }, #1
	sqrshru	z31.b, { z0.s - z3.s }, #1
	sqrshru	z0.b, { z28.s - z31.s }, #1
	sqrshru	z0.b, { z0.s - z3.s }, #32
	sqrshru	z6.b, { z12.s - z15.s }, #25

	sqrshru	z0.h, { z0.d - z3.d }, #1
	sqrshru	z31.h, { z0.d - z3.d }, #1
	sqrshru	z0.h, { z28.d - z31.d }, #1
	sqrshru	z0.h, { z0.d - z3.d }, #64
	sqrshru	z25.h, { z20.d - z23.d }, #50

	uqrshr	z0.h, { z0.s - z1.s }, #1
	uqrshr	z31.h, { z0.s - z1.s }, #1
	uqrshr	z0.h, { z30.s - z31.s }, #1
	uqrshr	z0.h, { z0.s - z1.s }, #16
	uqrshr	z14.h, { z22.s - z23.s }, #7

	uqrshr	z0.b, { z0.s - z3.s }, #1
	uqrshr	z31.b, { z0.s - z3.s }, #1
	uqrshr	z0.b, { z28.s - z31.s }, #1
	uqrshr	z0.b, { z0.s - z3.s }, #32
	uqrshr	z6.b, { z12.s - z15.s }, #25

	uqrshr	z0.h, { z0.d - z3.d }, #1
	uqrshr	z31.h, { z0.d - z3.d }, #1
	uqrshr	z0.h, { z28.d - z31.d }, #1
	uqrshr	z0.h, { z0.d - z3.d }, #64
	uqrshr	z25.h, { z20.d - z23.d }, #50

	// Invalid UQRSHR
	.inst 0xc13fd820
	.inst 0xc120d820
