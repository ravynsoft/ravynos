	sqrshrn	0, { z0.s - z3.s }, #1
	sqrshrn	z0.b, 0, #1

	sqrshrn	z0.b, { z1.s - z4.s }, #1
	sqrshrn	z0.b, { z2.s - z5.s }, #1
	sqrshrn	z0.b, { z3.s - z6.s }, #1
	sqrshrn	z0.b, { z0.s - z3.s }, #-1
	sqrshrn	z0.b, { z0.s - z3.s }, #0
	sqrshrn	z0.b, { z0.s - z3.s }, #33
	sqrshrn	z0.b, { z0.d - z3.d }, #1
	sqrshrn	z0.b, { z0.d - z3.d }, #65 // Double error
