	.section .foo,"a",@progbits
	.word	0xdeadbeef

	.abicalls
	.data
	.word	foo
