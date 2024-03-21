# Source file used to test the stw.n instruction

foo:
	stw.n	r4,0(r17)
	stw.n	r4,4(r17)
	stw.n	r4,0x1c(r17)
	stw.n	r4,0x3c(r17)
	stw.n	r4,0(r5)
	stw.n	r4,4(r5)
	stw.n	r4,0x1c(r5)
	stw.n	r4,0x3c(r5)
	stwz.n	zero,0(r16)
	stwz.n	r0,0(r16)
	stwz.n	zero,252(r16)
	stwz.n	zero,252(r7)
