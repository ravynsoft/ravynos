# Test read/write %cfr instructions
	.text
	wr	%o1, %l6, %cfr
	wr	%g1, 0x234, %cfr
	rd	%cfr, %g5
	rd	%cfr, %o3
	rd	%cfr, %i2
	rd	%cfr, %l4
