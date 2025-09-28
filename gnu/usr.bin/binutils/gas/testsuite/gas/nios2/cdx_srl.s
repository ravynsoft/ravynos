# Source file used to test the srl.n and srli.n instructions
	
foo:
	srl.n	r4,r4,r4
	srli.n	r4,r4,0x1
	srli.n	r4,r4,0x2
	srli.n	r4,r4,0x3
	srli.n	r4,r4,0x8
	srli.n	r4,r4,0xc
	srli.n	r4,r4,0x10
	srli.n	r4,r4,0x18
	srli.n	r4,r4,0x1f
	srl.n	r7,r7,r16
	srl.n	r16,r16,r7
