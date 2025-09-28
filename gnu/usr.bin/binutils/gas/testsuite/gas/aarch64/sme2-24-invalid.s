	bfcvt	0, { z0.s - z1.s }
	bfcvt	z0.h, 0

	bfcvt	z0.h, { z1.s - z2.s }
	bfcvtn	z0.h, { z0.s - z2.s }
	bfcvt	z0.h, { z0.s - z3.s }
	bfcvtn	z0.s, { z0.s - z3.s }
	bfcvt	z0.s, { z0.h - z3.h }

	fcvt	z0.s, { z0.h - z1.h }
	fcvt	z0.s, { z0.s - z1.s }
	fcvt	z0.d, { z0.s - z1.s }
	fcvt	z0.h, { z1.s - z2.s }
