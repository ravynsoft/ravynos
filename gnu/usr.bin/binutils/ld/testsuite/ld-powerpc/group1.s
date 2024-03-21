	.abiversion 2
	.section .text,"axG",%progbits,foo,comdat
	.globl	foo
	.type	foo,%function
foo:
0:
	nop
	.localentry foo,.-0b
	blr
	.size	foo,.-0b
