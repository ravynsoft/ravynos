	sqcvt	z0.h, { z0.s - z1.s }
	sqcvt	z31.h, { z0.s - z1.s }
	sqcvt	z0.h, { z30.s - z31.s }
	sqcvt	z19.h, { z14.s - z15.s }

	sqcvt	z0.b, { z0.s - z3.s }
	sqcvt	z31.b, { z0.s - z3.s }
	sqcvt	z0.b, { z28.s - z31.s }
	sqcvt	z11.b, { z20.s - z23.s }

	sqcvt	z0.h, { z0.d - z3.d }
	sqcvt	z31.h, { z0.d - z3.d }
	sqcvt	z0.h, { z28.d - z31.d }
	sqcvt	z22.h, { z4.d - z7.d }

	sqcvtu	z0.h, { z0.s - z1.s }
	sqcvtu	z31.h, { z0.s - z1.s }
	sqcvtu	z0.h, { z30.s - z31.s }
	sqcvtu	z19.h, { z14.s - z15.s }

	sqcvtu	z0.b, { z0.s - z3.s }
	sqcvtu	z31.b, { z0.s - z3.s }
	sqcvtu	z0.b, { z28.s - z31.s }
	sqcvtu	z11.b, { z20.s - z23.s }

	sqcvtu	z0.h, { z0.d - z3.d }
	sqcvtu	z31.h, { z0.d - z3.d }
	sqcvtu	z0.h, { z28.d - z31.d }
	sqcvtu	z22.h, { z4.d - z7.d }

	uqcvt	z0.h, { z0.s - z1.s }
	uqcvt	z31.h, { z0.s - z1.s }
	uqcvt	z0.h, { z30.s - z31.s }
	uqcvt	z19.h, { z14.s - z15.s }

	uqcvt	z0.b, { z0.s - z3.s }
	uqcvt	z31.b, { z0.s - z3.s }
	uqcvt	z0.b, { z28.s - z31.s }
	uqcvt	z11.b, { z20.s - z23.s }

	uqcvt	z0.h, { z0.d - z3.d }
	uqcvt	z31.h, { z0.d - z3.d }
	uqcvt	z0.h, { z28.d - z31.d }
	uqcvt	z22.h, { z4.d - z7.d }
