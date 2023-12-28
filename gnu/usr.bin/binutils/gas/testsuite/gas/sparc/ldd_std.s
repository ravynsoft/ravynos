# Test ldd/ldda/std/stda
	.text
	ldd	[%g3], %o0
	ldda	[%g3] 0x04, %o2
	std	%o0, [%g3]
	stda	%o2, [%g3] 0x04
