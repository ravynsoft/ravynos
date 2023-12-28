# Test ldx/ldxa/stx/stxa
	.text
	ldx	[%g3], %o0
	ldxa	[%g3] 0x04, %o2
	ldx	[%g3], %fsr
	stx	%o0, [%g3]
	stxa	%o2, [%g3] 0x04
	stx	%fsr, [%g3]
