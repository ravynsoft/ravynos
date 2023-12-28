# Source file used to test the sll.n and slli.n instructions
	
foo:
	sll.n	r4,r4,r4
	slli.n	r4,r4,0x1
	slli.n	r4,r4,0x2
	slli.n	r4,r4,0x3
	slli.n	r4,r4,0x8
	slli.n	r4,r4,0xc
	slli.n	r4,r4,0x10
	slli.n	r4,r4,0x18
	slli.n	r4,r4,0x1f
	sll.n	r7,r7,r16
	sll.n	r16,r16,r7
