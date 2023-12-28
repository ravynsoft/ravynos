# Test PAUSE instruction
	.text
	wr	%g1, %g2, %pause
	wr	%o4, 0x234, %pause
	pause	8
	pause	300
	pause	0x234
