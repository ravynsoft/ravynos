	.text
foo:
	call	func1@PLT
	pushl	__ehdr_start@GOT(%ebx)

	.section .bar,"aw",@progbits
	.p2align 2
	.dc.a	data1
	.dc.a	__ehdr_start

	.section .foo,"aw",@progbits
	.p2align 2
	.dc.a	data1
	.dc.a	__ehdr_start
