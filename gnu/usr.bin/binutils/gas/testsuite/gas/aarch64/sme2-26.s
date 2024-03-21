	sqcvtn	z0.b, { z0.s - z3.s }
	sqcvtn	z31.b, { z0.s - z3.s }
	sqcvtn	z0.b, { z28.s - z31.s }
	sqcvtn	z11.b, { z20.s - z23.s }

	sqcvtn	z0.h, { z0.d - z3.d }
	sqcvtn	z31.h, { z0.d - z3.d }
	sqcvtn	z0.h, { z28.d - z31.d }
	sqcvtn	z22.h, { z4.d - z7.d }

	sqcvtun	z0.b, { z0.s - z3.s }
	sqcvtun	z31.b, { z0.s - z3.s }
	sqcvtun	z0.b, { z28.s - z31.s }
	sqcvtun	z11.b, { z20.s - z23.s }

	sqcvtun	z0.h, { z0.d - z3.d }
	sqcvtun	z31.h, { z0.d - z3.d }
	sqcvtun	z0.h, { z28.d - z31.d }
	sqcvtun	z22.h, { z4.d - z7.d }

	uqcvtn	z0.b, { z0.s - z3.s }
	uqcvtn	z31.b, { z0.s - z3.s }
	uqcvtn	z0.b, { z28.s - z31.s }
	uqcvtn	z11.b, { z20.s - z23.s }

	uqcvtn	z0.h, { z0.d - z3.d }
	uqcvtn	z31.h, { z0.d - z3.d }
	uqcvtn	z0.h, { z28.d - z31.d }
	uqcvtn	z22.h, { z4.d - z7.d }
