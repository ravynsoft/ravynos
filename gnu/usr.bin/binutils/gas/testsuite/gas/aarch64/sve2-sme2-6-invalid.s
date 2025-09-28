	sqrshrn	0, { z0.s - z1.s }, #1
	sqrshrn	z0.h, 0, #1

	sqrshrn	z0.h, { z1.s - z2.s }, #1
	sqrshrn	z0.h, { z0.s - z2.s }, #1
	sqrshrn	z0.h, { z0.s - z3.s }, #1
	sqrshrn	z0.h, { z0.s - z1.s }, #0
	sqrshrn	z0.h, { z0.s - z1.s }, #17
	sqrshrn	z0.h, { z0.s - z1.s }, x0

	sqrshrn	z0.b, { z0.h - z1.h }, #1
	sqrshrn	z0.s, { z0.d - z1.d }, #1

	movprfx z0, z4; sqrshrn z0.h, { z2.s - z3.s }, #1
