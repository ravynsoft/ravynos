# Source file used to test the stwsp.n instruction

foo:
	stwsp.n	r4,0(sp)
	stwsp.n	r4,4(sp)
	stwsp.n	r4,0x3c(sp)
	stwsp.n	r4,0x7c(sp)
