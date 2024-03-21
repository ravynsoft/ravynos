	bfcvt	z0.h, { z0.s - z1.s }
	bfcvt	z31.h, { z0.s - z1.s }
	bfcvt	z0.h, { z30.s - z31.s }
	bfcvt	z14.h, { z20.s - z21.s }

	bfcvtn	z0.h, { z0.s - z1.s }
	bfcvtn	z31.h, { z0.s - z1.s }
	bfcvtn	z0.h, { z30.s - z31.s }
	bfcvtn	z26.h, { z14.s - z15.s }

	fcvt	z0.h, { z0.s - z1.s }
	fcvt	z31.h, { z0.s - z1.s }
	fcvt	z0.h, { z30.s - z31.s }
	fcvt	z29.h, { z6.s - z7.s }

	fcvtn	z0.h, { z0.s - z1.s }
	fcvtn	z31.h, { z0.s - z1.s }
	fcvtn	z0.h, { z30.s - z31.s }
	fcvtn	z29.h, { z6.s - z7.s }
