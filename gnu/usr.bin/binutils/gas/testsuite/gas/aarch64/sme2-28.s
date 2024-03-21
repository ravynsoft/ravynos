	sqrshrn	z0.b, { z0.s - z3.s }, #1
	sqrshrn	z31.b, { z0.s - z3.s }, #1
	sqrshrn	z0.b, { z28.s - z31.s }, #1
	sqrshrn	z0.b, { z0.s - z3.s }, #32
	sqrshrn	z6.b, { z12.s - z15.s }, #25

	sqrshrn	z0.h, { z0.d - z3.d }, #1
	sqrshrn	z31.h, { z0.d - z3.d }, #1
	sqrshrn	z0.h, { z28.d - z31.d }, #1
	sqrshrn	z0.h, { z0.d - z3.d }, #64
	sqrshrn	z25.h, { z20.d - z23.d }, #50

	sqrshrun z0.b, { z0.s - z3.s }, #1
	sqrshrun z31.b, { z0.s - z3.s }, #1
	sqrshrun z0.b, { z28.s - z31.s }, #1
	sqrshrun z0.b, { z0.s - z3.s }, #32
	sqrshrun z6.b, { z12.s - z15.s }, #25

	uqrshrn	z0.b, { z0.s - z3.s }, #1
	uqrshrn	z31.b, { z0.s - z3.s }, #1
	uqrshrn	z0.b, { z28.s - z31.s }, #1
	uqrshrn	z0.b, { z0.s - z3.s }, #32
	uqrshrn	z6.b, { z12.s - z15.s }, #25

	uqrshrn	z0.h, { z0.d - z3.d }, #1
	uqrshrn	z31.h, { z0.d - z3.d }, #1
	uqrshrn	z0.h, { z28.d - z31.d }, #1
	uqrshrn	z0.h, { z0.d - z3.d }, #64
	uqrshrn	z25.h, { z20.d - z23.d }, #50
