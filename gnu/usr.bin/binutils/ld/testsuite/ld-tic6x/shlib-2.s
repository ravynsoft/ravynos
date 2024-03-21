	.weak a
	.weak b

	.text
	.nocmp
	.align	2
	.global	sub
	.type	sub, @function
sub:
		sub	.d2	B15, 16, B15
		stw	.d2t1	A4, *+B15(12)
		ldw	.d2t1	*+B15(12), A0
		nop	4
		add	.d1	A0, 10, A0
		mv	.d1	A0, A4
		add	.d2	B15, 16, B15
		ret	.s2	B3
		ldw .d2t2 *+B14($GOT(a)), B6
		ldw .d2t2 *+B14($GOT(b)), B7
		ldw .d2t2 *+B14($GOT(c)), B8
		ldw .d2t2 *+B14(c), B9
		nop	1
	.size	sub, .-sub
	.section	.note.GNU-stack
