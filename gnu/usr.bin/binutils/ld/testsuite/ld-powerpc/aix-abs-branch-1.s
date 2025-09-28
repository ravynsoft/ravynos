	.globl	foo
	.csect	foo[PR]
foo:
	bl	bar - 0x3000
	lwz	1,80(1)
	bl	bar + 0x1000
	.ifeq	size - 32
	lwz	2,20(1)
	.else
	ld	2,40(1)
	.endif
	bl	bar + 0x2800
	nop
	bl	bar
