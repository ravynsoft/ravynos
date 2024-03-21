# Test program for movia reg, immed32 macro

foo:
	movia r2, 0x80808080
	movia r3, sym + 0x80000000
	movia r4, sym - 0x7fffffff
	movia r2, 0xfffffff0
