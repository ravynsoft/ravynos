	sqrshrn	z0.h, { z0.s - z1.s }, #1
	sqrshrn	z31.h, { z0.s - z1.s }, #1
	sqrshrn	z0.h, { z30.s - z31.s }, #1
	sqrshrn	z0.h, { z0.s - z1.s }, #16
	sqrshrn	z1.h, { z26.s - z27.s }, #14

	sqrshrun z0.h, { z0.s - z1.s }, #1
	sqrshrun z31.h, { z0.s - z1.s }, #1
	sqrshrun z0.h, { z30.s - z31.s }, #1
	sqrshrun z0.h, { z0.s - z1.s }, #16
	sqrshrun z15.h, { z6.s - z7.s }, #9

	uqrshrn	z0.h, { z0.s - z1.s }, #1
	uqrshrn	z31.h, { z0.s - z1.s }, #1
	uqrshrn	z0.h, { z30.s - z31.s }, #1
	uqrshrn	z0.h, { z0.s - z1.s }, #16
	uqrshrn	z18.h, { z2.s - z3.s }, #6
