	sqcvtn	0, { z0.s - z1.s }
	sqcvtn	z0.h, 0

	sqcvtn	z0.h, { z1.s - z2.s }
	sqcvtun	z0.h, { z0.s - z2.s }
	sqcvtn	z0.h, { z0.s - z3.s }
	sqcvtun	z0.s, { z0.s - z3.s }
	sqcvtn	z0.s, { z0.h - z3.h }
	sqcvtun	z0.b, { z0.h - z1.h }
	sqcvtn	z0.s, { z0.d - z1.d }

	movprfx z0, z4; sqcvtn z0.h, { z2.s - z3.s }
