
	| Check that non-zero displacements work for movem.
	.text
	movem.l -24(%a6),#1148
	movem.l #1148,16(%a6)
	movem.l -24(%a6),%d0-%d7/%a0-%a1
	movem.l %d0-%d7/%a0-%a1,16(%a6)
	.p2align 4
