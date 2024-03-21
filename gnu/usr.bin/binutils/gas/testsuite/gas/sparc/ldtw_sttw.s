# Test ldtw/ldtwa/sttw/sttwa
	.text
	ldtw	[%g3], %o0
	ldtwa	[%g3] 0x04, %o2
	ldtwa	[%g3] %asi, %o4
	sttw	%o0, [%g3]
	sttwa	%o2, [%g3] 0x04
	sttwa	%o4, [%g3] %asi
