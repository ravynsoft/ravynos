# Source file used to test the ldbu.n instruction

foo:
	ldbu.n	r4,0(r17)
	ldbu.n	r4,4(r17)
	ldbu.n	r4,7(r17)
	ldbu.n	r4,0xf(r17)
	ldbu.n	r4,0(r5)
	ldbu.n	r4,4(r5)
	ldbu.n	r4,7(r5)
	ldbu.n	r4,0xf(r5)
