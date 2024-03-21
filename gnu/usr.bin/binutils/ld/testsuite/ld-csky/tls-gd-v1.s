	.global	__tls_get_addr
	.text
	.align	2
	.global	foo
	.type	foo, @function
foo:
	subi	sp, sp, 8
	st.w	gb, (sp)
	st.w	r15, (sp, 4)
	bsr	.L2
.L2:
	lrw	gb, .L2@GOTPC
	addu	gb, gb, r15
	lrw	r7, __tls_get_addr@PLT
	addu	r7, r7, gb
	ld.w	r7, (r7)
	bsr	.LTLS0
.LTLS0:
	lrw	r2, var@TLSGD32
	addu	r2, r15
	jsr		r7
	ld.w	r2, (r2)
	ld.w	r15, (sp, 4)
	ld.w	gb, (sp)
	addi	sp, sp, 8
	jmp	r15
