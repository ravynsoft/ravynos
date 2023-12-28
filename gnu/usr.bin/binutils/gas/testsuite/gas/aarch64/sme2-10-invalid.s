	fmax	0, { z0.h - z1.h }, z0.h
	fmax	{ z0.h - z1.h }, 0, z0.h
	fmax	{ z0.h - z1.h }, { z0.h - z1.h }, 0

	fmax	{ z0.b - z1.b }, { z0.b - z1.b }, z0.b
	fmax	{ z0.h - z1.h }, { z0.h - z2.h }, z0.h
	fmax	{ z0.h - z1.h }, { z0.h - z3.h }, z0.h
	fmax	{ z0.h - z1.h }, { z0.h, z8.h }, z0.h
	fmax	{ z0.h - z2.h }, { z0.h - z2.h }, z0.h
	fmax	{ z0.h - z1.h }, { z2.h - z3.h }, z0.h
	fmax	{ z1.h - z2.h }, { z1.h - z2.h }, z0.h
	fmax	{ z31.h, z0.h }, { z31.h, z0.h }, z0.h
	fmax	{ z0.h - z1.h }, { z0.h - z1.h }, z16.h
	fmax	{ z0.h - z1.h }, { z0.h - z1.h }, z31.h

	fmax	{ z0.b - z3.b }, { z0.b - z3.b }, z0.b
	fmax	{ z0.h - z3.h }, { z0.h - z2.h }, z0.h
	fmax	{ z0.h - z3.h }, { z0.h - z1.h }, z0.h
	fmax	{ z0.h - z3.h }, { z2.h - z5.h }, z0.h
	fmax	{ z1.h - z4.h }, { z1.h - z4.h }, z0.h
	fmax	{ z2.h - z5.h }, { z2.h - z5.h }, z0.h
	fmax	{ z3.h - z6.h }, { z3.h - z6.h }, z0.h
	fmax	{ z31.h, z0.h, z1.h, z2.h }, { z31.h, z0.h, z1.h, z2.h }, z0.h
	fmax	{ z0.h - z3.h }, { z0.h - z3.h }, z16.h
	fmax	{ z0.h - z3.h }, { z0.h - z3.h }, z31.h

	fmax	{ z0.b - z1.b }, { z0.b - z1.b }, { z0.b - z1.b }
	fmax	{ z0.h - z1.h }, { z0.h - z2.h }, { z0.h - z1.h }
	fmax	{ z0.h - z1.h }, { z0.h - z3.h }, { z0.h - z1.h }
	fmax	{ z0.h - z1.h }, { z0.h - z1.h }, { z0.h - z2.h }
	fmax	{ z0.h - z1.h }, { z0.h - z1.h }, { z0.h - z3.h }
	fmax	{ z0.h - z2.h }, { z0.h - z2.h }, { z0.h - z1.h }
	fmax	{ z0.h - z1.h }, { z2.h - z3.h }, { z0.h - z1.h }
	fmax	{ z1.h - z2.h }, { z1.h - z2.h }, { z0.h - z1.h }
	fmax	{ z0.h - z1.h }, { z0.h - z1.h }, { z1.h - z2.h }
	fmax	{ z31.h, z0.h }, { z31.h, z0.h }, { z0.h - z1.h }
	fmax	{ z0.h - z1.h }, { z0.h - z1.h }, { z31.h, z0.h }

	fmax	{ z0.b - z3.b }, { z0.b - z3.b }, { z0.b - z3.b }
	fmax	{ z0.h - z3.h }, { z0.h - z1.h }, { z0.h - z3.h }
	fmax	{ z0.h - z3.h }, { z0.h - z2.h }, { z0.h - z3.h }
	fmax	{ z0.h - z3.h }, { z0.h - z3.h }, { z0.h - z1.h }
	fmax	{ z0.h - z3.h }, { z0.h - z3.h }, { z0.h - z2.h }
	fmax	{ z0.h - z3.h }, { z4.h - z7.h }, { z0.h - z3.h }
	fmax	{ z1.h - z4.h }, { z1.h - z4.h }, { z0.h - z3.h }
	fmax	{ z2.h - z5.h }, { z2.h - z5.h }, { z0.h - z3.h }
	fmax	{ z3.h - z6.h }, { z3.h - z6.h }, { z0.h - z3.h }
	fmax	{ z0.h - z3.h }, { z0.h - z3.h }, { z1.h - z4.h }
	fmax	{ z0.h - z3.h }, { z0.h - z3.h }, { z2.h - z5.h }
	fmax	{ z0.h - z3.h }, { z0.h - z3.h }, { z3.h - z6.h }
