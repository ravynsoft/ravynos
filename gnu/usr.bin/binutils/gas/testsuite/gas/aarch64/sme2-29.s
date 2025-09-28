	sunpk	{ z0.h, z1.h }, z0.b
	sunpk	{ z30.h, z31.h }, z0.b
	sunpk	{ z0.h, z1.h }, z31.b

	sunpk	{ z0.h - z3.h }, { z0.b - z1.b }
	sunpk	{ z28.h - z31.h }, { z0.b - z1.b }
	sunpk	{ z0.h - z3.h }, { z30.b - z31.b }

	sunpk	{ z0.s, z1.s }, z0.h
	sunpk	{ z30.s, z31.s }, z0.h
	sunpk	{ z0.s, z1.s }, z31.h

	sunpk	{ z0.s - z3.s }, { z0.h - z1.h }
	sunpk	{ z28.s - z31.s }, { z0.h - z1.h }
	sunpk	{ z0.s - z3.s }, { z30.h - z31.h }

	sunpk	{ z0.d, z1.d }, z0.s
	sunpk	{ z30.d, z31.d }, z0.s
	sunpk	{ z0.d, z1.d }, z31.s

	sunpk	{ z0.d - z3.d }, { z0.s - z1.s }
	sunpk	{ z28.d - z31.d }, { z0.s - z1.s }
	sunpk	{ z0.d - z3.d }, { z30.s - z31.s }

	uunpk	{ z0.h, z1.h }, z0.b
	uunpk	{ z30.h, z31.h }, z0.b
	uunpk	{ z0.h, z1.h }, z31.b

	uunpk	{ z0.h - z3.h }, { z0.b - z1.b }
	uunpk	{ z28.h - z31.h }, { z0.b - z1.b }
	uunpk	{ z0.h - z3.h }, { z30.b - z31.b }

	uunpk	{ z0.s, z1.s }, z0.h
	uunpk	{ z30.s, z31.s }, z0.h
	uunpk	{ z0.s, z1.s }, z31.h

	uunpk	{ z0.s - z3.s }, { z0.h - z1.h }
	uunpk	{ z28.s - z31.s }, { z0.h - z1.h }
	uunpk	{ z0.s - z3.s }, { z30.h - z31.h }

	uunpk	{ z0.d, z1.d }, z0.s
	uunpk	{ z30.d, z31.d }, z0.s
	uunpk	{ z0.d, z1.d }, z31.s

	uunpk	{ z0.d - z3.d }, { z0.s - z1.s }
	uunpk	{ z28.d - z31.d }, { z0.s - z1.s }
	uunpk	{ z0.d - z3.d }, { z30.s - z31.s }
