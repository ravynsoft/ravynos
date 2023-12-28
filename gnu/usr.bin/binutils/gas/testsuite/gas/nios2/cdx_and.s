# Source file used to test the and.n and andi.n instructions
	
foo:
	and.n	r4,r4,r4
	andi.n	r4,r4,0x1
	andi.n	r4,r4,0x2
	andi.n	r4,r4,0x3
	andi.n	r4,r4,0x4
	andi.n	r4,r4,0x8
	andi.n	r4,r4,0xf
	andi.n	r4,r4,0x10
	andi.n	r4,r4,0x1f
	andi.n	r4,r4,0x20
	andi.n	r4,r4,0x3f
	andi.n	r4,r4,0x7f
	andi.n	r4,r4,0x80
	andi.n	r4,r4,0xff
	andi.n	r4,r4,0x7ff
	andi.n	r4,r4,0xff00
	andi.n	r4,r4,0xffff
