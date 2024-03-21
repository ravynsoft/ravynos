	.text
	.global	__tls_get_addr
	.align	2
	.global	foo
	.type	foo, @function
foo:
	push	lr, gb
	lrw	t1, .L2@GOTPC
	grs	gb, .L2
.L2:
	addu	gb, gb, t1
	grs	a2, .LTLS0
	lrw	a3, __tls_get_addr@PLT
.LTLS0:
	lrw	a0, var@TLSGD32
	ldr.w	a3, (gb, a3 << 0)
	addu	a0, a0, a2
	jsr	a3
	ld.w	a0, (a0, 0)
	pop	lr, gb
