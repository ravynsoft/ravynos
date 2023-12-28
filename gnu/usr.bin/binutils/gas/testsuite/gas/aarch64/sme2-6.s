	cntp	x0, pn0.b, vlx2
	CNTP	X0, PN0.B, VLx2
	cntp	x30, pn0.b, vlx2
	cntp	xzr, pn0.b, vlx2
	cntp	x0, pn15.b, vlx2
	cntp	x0, pn0.b, vlx4
	CNTP	X11, PN13.b, VLx4

	cntp	x0, pn0.h, vlx2
	CNTP	X0, PN0.H, VLx2
	cntp	x30, pn0.h, vlx2
	cntp	xzr, pn0.h, vlx2
	cntp	x0, pn15.h, vlx2
	cntp	x0, pn0.h, vlx4
	CNTP	X20, PN9.h, VLx2

	cntp	x0, pn0.s, vlx2
	CNTP	X0, PN0.s, VLx2
	cntp	x30, pn0.s, vlx2
	cntp	xzr, pn0.s, vlx2
	cntp	x0, pn15.s, vlx2
	cntp	x0, pn0.s, vlx4
	CNTP	X15, PN8.s, VLx4

	cntp	x0, pn0.d, vlx2
	CNTP	X0, PN0.d, VLx2
	cntp	x30, pn0.d, vlx2
	cntp	xzr, pn0.d, vlx2
	cntp	x0, pn15.d, vlx2
	cntp	x0, pn0.d, vlx4
	CNTP	X4, PN5.d, VLx2

	pext	p0.b, pn8[0]
	PEXT	P0.B, PN8[0]
	pext	p15.b, pn8[0]
	pext	p0.b, pn15[0]
	pext	p0.b, pn8[3]
	pext	p4.b, pn11[2]

	pext	p0.h, pn8[0]
	PEXT	P0.H, PN8[0]
	pext	p15.h, pn8[0]
	pext	p0.h, pn15[0]
	pext	p0.h, pn8[3]
	pext	p5.h, pn14[1]

	pext	p0.s, pn8[0]
	PEXT	P0.S, PN8[0]
	pext	p15.s, pn8[0]
	pext	p0.s, pn15[0]
	pext	p0.s, pn8[3]
	pext	p6.s, pn10[2]

	pext	p0.d, pn8[0]
	PEXT	P0.D, PN8[0]
	pext	p15.d, pn8[0]
	pext	p0.d, pn15[0]
	pext	p0.d, pn8[3]
	pext	p7.d, pn9[1]

	pext	{ p0.b, p1.b }, pn8[0]
	pext	{ p0.b - p1.b }, pn8[0]
	PEXT	{ P0.B - P1.B }, PN8[0]
	pext	{ p14.b - p15.b }, pn8[0]
	pext	{ p15.b, p0.b }, pn8[0]
	pext	{ p15.b - p0.b }, pn8[0]
	pext	{ p0.b - p1.b }, pn15[0]
	pext	{ p0.b - p1.b }, pn8[1]
	pext	{ p7.b - p8.b }, pn12[0]

	pext	{ p0.h, p1.h }, pn8[0]
	pext	{ p0.h - p1.h }, pn8[0]
	PEXT	{ P0.H - P1.H }, PN8[0]
	pext	{ p14.h - p15.h }, pn8[0]
	pext	{ p15.h, p0.h }, pn8[0]
	pext	{ p15.h - p0.h }, pn8[0]
	pext	{ p0.h - p1.h }, pn15[0]
	pext	{ p0.h - p1.h }, pn8[1]
	pext	{ p2.h - p3.h }, pn14[0]

	pext	{ p0.s, p1.s }, pn8[0]
	pext	{ p0.s - p1.s }, pn8[0]
	PEXT	{ P0.S - P1.S }, PN8[0]
	pext	{ p14.s - p15.s }, pn8[0]
	pext	{ p15.s, p0.s }, pn8[0]
	pext	{ p15.s - p0.s }, pn8[0]
	pext	{ p0.s - p1.s }, pn15[0]
	pext	{ p0.s - p1.s }, pn8[1]
	pext	{ p5.s - p6.s }, pn13[0]

	pext	{ p0.d, p1.d }, pn8[0]
	pext	{ p0.d - p1.d }, pn8[0]
	PEXT	{ P0.D - P1.D }, PN8[0]
	pext	{ p14.d - p15.d }, pn8[0]
	pext	{ p15.d, p0.d }, pn8[0]
	pext	{ p15.d - p0.d }, pn8[0]
	pext	{ p0.d - p1.d }, pn15[0]
	pext	{ p0.d - p1.d }, pn8[1]
	pext	{ p12.d - p13.d }, pn9[0]

	ptrue	pn8.b
	ptrue	pn11.b
	ptrue	pn15.b
	ptrue	pn8.h
	ptrue	pn9.h
	ptrue	pn15.h
	ptrue	pn8.s
	ptrue	pn14.s
	ptrue	pn15.s
	ptrue	pn8.d
	ptrue	pn12.d
	ptrue	pn15.d

	sel	{ z0.b - z1.b }, pn8, { z0.b - z1.b }, { z0.b - z1.b }
	sel	{ z30.b - z31.b }, pn8, { z0.b - z1.b }, { z0.b - z1.b }
	sel	{ z0.b - z1.b }, pn15, { z0.b - z1.b }, { z0.b - z1.b }
	sel	{ z0.b - z1.b }, pn8, { z30.b - z31.b }, { z0.b - z1.b }
	sel	{ z0.b - z1.b }, pn8, { z0.b - z1.b }, { z30.b - z31.b }
	sel	{ z2.b - z3.b }, pn12, { z6.b - z7.b }, { z10.b - z11.b }

	sel	{ z0.h - z1.h }, pn8, { z0.h - z1.h }, { z0.h - z1.h }
	sel	{ z30.h - z31.h }, pn8, { z0.h - z1.h }, { z0.h - z1.h }
	sel	{ z0.h - z1.h }, pn15, { z0.h - z1.h }, { z0.h - z1.h }
	sel	{ z0.h - z1.h }, pn8, { z30.h - z31.h }, { z0.h - z1.h }
	sel	{ z0.h - z1.h }, pn8, { z0.h - z1.h }, { z30.h - z31.h }
	sel	{ z12.h - z13.h }, pn9, { z14.h - z15.h }, { z16.h - z17.h }

	sel	{ z0.s - z1.s }, pn8, { z0.s - z1.s }, { z0.s - z1.s }
	sel	{ z30.s - z31.s }, pn8, { z0.s - z1.s }, { z0.s - z1.s }
	sel	{ z0.s - z1.s }, pn15, { z0.s - z1.s }, { z0.s - z1.s }
	sel	{ z0.s - z1.s }, pn8, { z30.s - z31.s }, { z0.s - z1.s }
	sel	{ z0.s - z1.s }, pn8, { z0.s - z1.s }, { z30.s - z31.s }
	sel	{ z18.s - z19.s }, pn11, { z22.s - z23.s }, { z24.s - z25.s }

	sel	{ z0.d - z1.d }, pn8, { z0.d - z1.d }, { z0.d - z1.d }
	sel	{ z30.d - z31.d }, pn8, { z0.d - z1.d }, { z0.d - z1.d }
	sel	{ z0.d - z1.d }, pn15, { z0.d - z1.d }, { z0.d - z1.d }
	sel	{ z0.d - z1.d }, pn8, { z30.d - z31.d }, { z0.d - z1.d }
	sel	{ z0.d - z1.d }, pn8, { z0.d - z1.d }, { z30.d - z31.d }
	sel	{ z8.d - z9.d }, pn14, { z26.d - z27.d }, { z28.d - z29.d }

	sel	{ z0.b - z3.b }, pn8, { z0.b - z3.b }, { z0.b - z3.b }
	sel	{ z28.b - z31.b }, pn8, { z0.b - z3.b }, { z0.b - z3.b }
	sel	{ z0.b - z3.b }, pn8, { z28.b - z31.b }, { z0.b - z3.b }
	sel	{ z0.b - z3.b }, pn8, { z0.b - z3.b }, { z28.b - z31.b }
	sel	{ z4.b - z7.b }, pn10, { z8.b - z11.b }, { z12.b - z15.b }

	sel	{ z0.h - z3.h }, pn8, { z0.h - z3.h }, { z0.h - z3.h }
	sel	{ z28.h - z31.h }, pn8, { z0.h - z3.h }, { z0.h - z3.h }
	sel	{ z0.h - z3.h }, pn8, { z28.h - z31.h }, { z0.h - z3.h }
	sel	{ z0.h - z3.h }, pn8, { z0.h - z3.h }, { z28.h - z31.h }
	sel	{ z8.h - z11.h }, pn10, { z16.h - z19.h }, { z20.h - z23.h }

	sel	{ z0.s - z3.s }, pn8, { z0.s - z3.s }, { z0.s - z3.s }
	sel	{ z28.s - z31.s }, pn8, { z0.s - z3.s }, { z0.s - z3.s }
	sel	{ z0.s - z3.s }, pn8, { z28.s - z31.s }, { z0.s - z3.s }
	sel	{ z0.s - z3.s }, pn8, { z0.s - z3.s }, { z28.s - z31.s }
	sel	{ z16.s - z19.s }, pn10, { z20.s - z23.s }, { z24.s - z27.s }

	sel	{ z0.d - z3.d }, pn8, { z0.d - z3.d }, { z0.d - z3.d }
	sel	{ z28.d - z31.d }, pn8, { z0.d - z3.d }, { z0.d - z3.d }
	sel	{ z0.d - z3.d }, pn8, { z28.d - z31.d }, { z0.d - z3.d }
	sel	{ z0.d - z3.d }, pn8, { z0.d - z3.d }, { z28.d - z31.d }
	sel	{ z20.d - z23.d }, pn10, { z4.d - z7.d }, { z8.d - z11.d }
