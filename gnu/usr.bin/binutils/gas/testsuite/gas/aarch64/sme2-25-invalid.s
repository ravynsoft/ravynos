	sqcvt	0, { z0.s - z1.s }
	sqcvt	z0.h, 0

	sqcvt	z0.s, { z0.s - z1.s }
	sqcvt	z0.b, { z0.d - z1.d }
	sqcvt	z0.s, { z0.d - z1.d }

	sqcvt	z0.s, { z0.s - z3.s }
	sqcvt	z0.b, { z0.d - z3.d }
	sqcvt	z0.s, { z0.d - z3.d }

	sqcvt	z0.h, { z0.s - z2.s }
	sqcvt	z0.h, { z0.s - z3.s }
	sqcvt	z0.h, { z0.s, z8.s }
	sqcvt	z0.h, { z1.s - z2.s }
	sqcvt	z0.h, { z31.s, z0.s }

	sqcvt	z0.b, { z0.s - z1.s }
	sqcvt	z0.b, { z0.s - z2.s }
	sqcvt	z0.b, { z1.s - z4.s }
	sqcvt	z0.b, { z2.s - z5.s }
	sqcvt	z0.b, { z3.s - z6.s }

	sqcvt	z0.h, { z0.d - z1.d }
	sqcvt	z0.h, { z0.d - z2.d }
	sqcvt	z0.h, { z1.d - z4.d }
	sqcvt	z0.h, { z2.d - z5.d }
	sqcvt	z0.h, { z3.d - z6.d }
