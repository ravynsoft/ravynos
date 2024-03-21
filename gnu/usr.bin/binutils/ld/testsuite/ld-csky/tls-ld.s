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
	grs	a3, .LTLS0
.LTLS0:
	lrw	a0, .LANCHOR0@TLSLDM32
	addu	a0, a0, a3
	lrs.w	a3, [__tls_get_addr@PLT]
	jsr	a3
	lrw	a3, .LANCHOR0@TLSLDO32
	addu	a0, a0, a3
	pop	lr, gb
	.size	foo, .-foo
	.section	.tbss,"awT",@nobits
	.align	2
	.set	.LANCHOR0,. + 0
	.type	var, @object
	.size	var, 4
var:
	.fill 4, 1
