# Source file used to test the stb.n instruction

foo:
	stb.n	r4,0(r17)
	stb.n	r4,4(r17)
	stb.n	r4,0x7(r17)
	stb.n	r4,0xf(r17)
	stb.n	r4,0(r5)
	stb.n	r4,4(r5)
	stb.n	r4,0x7(r5)
	stb.n	r4,0xf(r5)
	stbz.n	zero,0(r16)
	stbz.n	r0,0(r16)
	stbz.n	zero,63(r16)
	stbz.n	zero,63(r7)
