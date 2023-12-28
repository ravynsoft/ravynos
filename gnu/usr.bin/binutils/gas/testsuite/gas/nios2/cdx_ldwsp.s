# Source file used to test the ldwsp.n instruction

foo:
	ldwsp.n	r4,0(sp)
	ldwsp.n	r4,4(sp)
	ldwsp.n	r4,0x3c(sp)
	ldwsp.n	r4,0x7c(sp)
