# sparc64 gotop

	.data
sym:	.word	0

	.text
foo:	sethi	%gdop_hix22(sym), %l1
	xor	%l1, %gdop_lox10(sym), %l1
	ldx	[%l7 + %l1], %l2, %gdop(sym)
