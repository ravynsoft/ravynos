	.text
	.align	2
	.global	foo
	.type	foo, @function
foo:
	push	l0, lr, gb
	lrw	t1, .L2@GOTPC
	grs	gb, .L2
.L2:
	addu	gb, gb, t1
	lrs.w	a3, [var1@GOT]
	ld.w	l0, (a3, 0)
	lrs.w	a3, [var2@GOT]
	ld.w	a3, (a3, 0)
	lrs.w	a0, [bar@PLT]
	addu	l0, l0, a3
	jsr	a0
	addu	a0, l0, a0
	pop	l0, lr, gb
