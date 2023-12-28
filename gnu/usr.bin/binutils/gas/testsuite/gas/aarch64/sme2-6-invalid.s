	cntp	0, pn0.b, vlx2
	cntp	x0, 0, vlx2
	cntp	x0, pn0.b, 0

	cntp	xsp, pn0.b, vlx2
	cntp	sp, pn0.b, vlx2
	cntp	w0, pn0.b, vlx2
	cntp	x0, p0.b, vlx2
	cntp	x0, pn16.b, vlx2
	cntp	x0, pn0.b, #0
	cntp	x0, pn0.b, vl
	cntp	x0, pn0.b, vlx3

	cntp	x0, pn0, vlx2
	cntp	x0, pn0.q, vlx2

	pext	0, pn8[0]
	pext	p0.b, 0

	pext	pn8.b, pn0[0]
	pext	z0.b, pn8[0]
	pext	p8.b, z8[0]
	pext	p8.b, x8
	pext	p8.b, p8[0]
	pext	p8.b, pn8
	pext	p8.b, pn8[-1]
	pext	p8.b, pn8[4]
	pext	p8.b, pn8[1 << 32]
	pext	p8.b, pn8.b[0]
	pext	p8.q, pn8[0]

	pext	{ p0.b - p2.b }, pn8[0]
	pext	{ p0 - p1 }, pn8[0]
	pext	{ p0.b - p1.b }, pn7[0]
	pext	{ p0.b - p1.b }, pn0[0]
	pext	{ p0.b - p1.b }, pn8
	pext	{ p0.b - p1.b }, p0[0]
	pext	{ p0.b - p1.b }, pn8.b[0]
	pext	{ p0.b - p1.b }, pn8[-1]
	pext	{ p0.b - p1.b }, pn8[2]
	pext	{ p0.b - p1.b }, pn8[1 << 32]

	ptrue	0

	ptrue	pn0.b
	ptrue	pn7.b
	ptrue	pn0.h
	ptrue	pn7.h
	ptrue	pn0.s
	ptrue	pn7.s
	ptrue	pn0.d
	ptrue	pn7.d
	ptrue	pn8.b, all
	ptrue	pn8.q

	sel	0, pn8, { z0.b - z1.b }, { z0.b - z1.b }
	sel	{ z0.b - z1.b }, 0, { z0.b - z1.b }, { z0.b - z1.b }
	sel	{ z0.b - z1.b }, pn8, 0, { z0.b - z1.b }
	sel	{ z0.b - z1.b }, pn8, { z0.b - z1.b }, 0

	sel	{ z1.b - z2.b }, pn8, { z0.b - z1.b }, { z0.b - z1.b }
	sel	{ z0.b - z1.b }, p8, { z0.b - z1.b }, { z0.b - z1.b }
	sel	{ z0.b - z1.b }, pn7, { z0.b - z1.b }, { z0.b - z1.b }
	sel	{ z0.b - z1.b }, pn8/z, { z0.b - z1.b }, { z0.b - z1.b }
	sel	{ z0.b - z1.b }, pn8/m, { z0.b - z1.b }, { z0.b - z1.b }
	sel	{ z0.b - z1.b }, pn0, { z0.b - z1.b }, { z0.b - z1.b }
	sel	{ z0.b - z1.b }, pn8, { z11.b - z12.b }, { z0.b - z1.b }
	sel	{ z0.b - z1.b }, pn8, { z0.b - z1.b }, { z17.b - z18.b }

	sel	{ z1.b - z4.b }, pn8, { z0.b - z3.b }, { z0.b - z3.b }
	sel	{ z10.b - z13.b }, pn8, { z0.b - z3.b }, { z0.b - z3.b }
	sel	{ z15.b - z18.b }, pn8, { z0.b - z3.b }, { z0.b - z3.b }
	sel	{ z0.b - z3.b }, pn8, { z1.b - z4.b }, { z0.b - z3.b }
	sel	{ z0.b - z3.b }, pn8, { z22.b - z25.b }, { z0.b - z3.b }
	sel	{ z0.b - z3.b }, pn8, { z27.b - z30.b }, { z0.b - z3.b }
	sel	{ z0.b - z3.b }, pn8, { z0.b - z3.b }, { z5.b - z8.b }
	sel	{ z0.b - z3.b }, pn8, { z0.b - z3.b }, { z14.b - z17.b }
	sel	{ z0.b - z3.b }, pn8, { z0.b - z3.b }, { z19.b - z22.b }

	sel	{ z0.b - z1.b }, pn8, { z0.b - z3.b }, { z0.b - z3.b }
	sel	{ z0.b - z3.b }, pn8, { z0.b - z1.b }, { z0.b - z3.b }
	sel	{ z0.b - z3.b }, pn8, { z0.b - z3.b }, { z0.b - z1.b }

	sel	{ z0.b - z2.b }, pn8, { z0.b - z3.b }, { z0.b - z3.b }
	sel	{ z0.b - z3.b }, pn8, { z0.b - z2.b }, { z0.b - z3.b }
	sel	{ z0.b - z3.b }, pn8, { z0.b - z3.b }, { z0.b - z2.b }

	sel	{ z0.b - z2.b }, pn8, { z0.b - z1.b }, { z0.b - z1.b }
	sel	{ z0.b - z1.b }, pn8, { z0.b - z2.b }, { z0.b - z1.b }
	sel	{ z0.b - z1.b }, pn8, { z0.b - z1.b }, { z0.b - z2.b }

	sel	{ z0.q - z1.q }, pn8, { z0.q - z1.q }, { z0.q - z1.q }
