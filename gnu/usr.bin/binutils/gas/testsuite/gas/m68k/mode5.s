
	| Test conversion of mode 5 addressing with a zero offset into mode 2.
	.text
	move.l 0(%a3),%d1
	move.l %d2,0(%a4)
	move.l 0(%a5),0(%a1)
	movem.l 0(%a6),%d0-%d7
	movem.l %d0-%d7,0(%a6)
	.p2align 4
