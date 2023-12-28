	fcvtzs	0, { z0.s - z1.s }
	fcvtzs	{ z0.s, z1.s }, 0

	fcvtzs	{ z0.s, z1.s }, { z0.h - z1.h }
	fcvtzs	{ z30.h, z31.h }, { z0.s - z1.s }
	fcvtzs	{ z0.d, z1.d }, { z30.d - z31.d }
	fcvtzs	{ z1.s, z2.s }, { z30.s - z31.s }
	fcvtzs	{ z0.s, z1.s }, { z29.s - z30.s }
