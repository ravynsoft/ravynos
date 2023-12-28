	.text
foo:
	call	func1@PLT
	add	__ehdr_start@GOTPCREL(%rip), %rax

	.section .bar,"aw",@progbits
	.p2align 3
	.dc.a	data1
	.dc.a	__ehdr_start

	.section .foo,"aw",@progbits
	.p2align 3
	.dc.a	data1
	.dc.a	__ehdr_start
