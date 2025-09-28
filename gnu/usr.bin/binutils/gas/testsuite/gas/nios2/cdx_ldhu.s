# Source file used to test the ldhu.n instruction

foo:
	ldhu.n	r4,0(r17)
	ldhu.n	r4,4(r17)
	ldhu.n	r4,0xe(r17)
	ldhu.n	r4,0x1e(r17)
	ldhu.n	r4,0(r5)
	ldhu.n	r4,4(r5)
	ldhu.n	r4,0xe(r5)
	ldhu.n	r4,0x1e(r5)
