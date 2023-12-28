	.abiversion 2
	.section .text,"axG",%progbits,foo,comdat
	.globl	foo
	.type	foo,%function
foo:
0:
	.localentry foo,1
	blr
	.size	foo,.-0b
