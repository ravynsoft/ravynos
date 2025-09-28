	sunpk	0, z0.b
	sunpk	{ z0.h, z1.h }, 0

	sunpk	z0.b, z0.b
	sunpk	{ z0.b, z1.b }, z0.b
	sunpk	{ z0.h, z1.h }, z0.h
	sunpk	{ z1.h, z2.h }, z0.b
	sunpk	{ z0.b, z2.b }, z0.b
	sunpk	{ z1.h - z3.h }, { z0.b - z1.b }
	sunpk	{ z2.h - z4.h }, { z0.b - z1.b }
	sunpk	{ z3.h - z5.h }, { z0.b - z1.b }
	sunpk	{ z0.s - z3.s }, z0.b
	sunpk	{ z0.s - z3.s }, { x0.s - x1.s }
	sunpk	{ z0.s - z3.s }, { z0.s - z3.s }
