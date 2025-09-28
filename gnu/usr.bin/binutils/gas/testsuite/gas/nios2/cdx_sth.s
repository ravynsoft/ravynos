# Source file used to test the sth.n instruction

foo:
	sth.n	r4,0(r17)
	sth.n	r4,4(r17)
	sth.n	r4,0xe(r17)
	sth.n	r4,0x1e(r17)
	sth.n	r4,0(r5)
	sth.n	r4,4(r5)
	sth.n	r4,0xe(r5)
	sth.n	r4,0x1e(r5)
