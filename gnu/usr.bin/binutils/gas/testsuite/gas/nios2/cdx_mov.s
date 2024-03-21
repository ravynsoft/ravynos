# Source file used to test the mov.n mov movi.n instructions
	
foo:
	mov.n	r4,r4
	movi.n	r4,0x0
	movi.n	r4,0x1
	movi.n	r4,0x3f
	movi.n	r4,0x7c
	movi.n	r4,0xff
	movi.n	r4,-2
	movi.n	r4,-1
