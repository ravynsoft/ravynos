# Test FLUSH instruction
	.text
	flush	[%g1 + %g2]
	flush	%g1 + %g2
	flush	[%g3]
	flush	%g3
	flush	[%g4 + 0x80]
	flush	%g4 + 0x80
	flush	[0x90 + %g5]
	flush	0x90 + %g5
