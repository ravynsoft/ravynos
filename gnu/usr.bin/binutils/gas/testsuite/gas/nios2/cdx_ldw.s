# Source file used to test the ldw.n instruction

foo:
	ldw.n	r4,0(r17)
	ldw.n	r4,4(r17)
	ldw.n	r4,0x1c(r17)
	ldw.n	r4,0x3c(r17)
	ldw.n	r4,0(r5)
	ldw.n	r4,4(r5)
	ldw.n	r4,0x1c(r5)
	ldw.n	r4,0x3c(r5)
