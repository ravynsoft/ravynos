	.text
	.nocmp
	.align	2
	.hidden	sub1
	.type	sub1, @function
sub1:
		ret .s2 B3
		nop	5

	.global	sub0
	.type	sub0, @function
sub0:
		sub	.d2	B15, 16, B15
		stw	.d2t2	B3, *+B15(12)
		stw	.d2t2	B14, *+B15(16)
		ldw	.d2t2	*+B14($DSBT_index(__c6xabi_DSBT_BASE)), B14
		call	.s2	(sub)
		call	.s2	(sub0)
		call	.s2	(sub1)
.L2:
		ldw	.d2t2	*+B15(12), B3
		ldw	.d2t2	*+B15(16), B14
		addk	.s2	16, B15
		nop	3
		ret	.s2	B3
		nop	5
	.size	sub0, .-sub0

	.global	a
	.section	.neardata,"aw",@progbits
	.align	2
	.type	a, @object
	.size	a, 4
a:
	.long	sub0
	.weak	g1
	.global g2
	.type	g2, @object
	.size	g2, 4
g2:
	.long	g1

	.hidden c
	.scomm	c,4,4
	.section	.note.GNU-stack
