	.set	mips16
	.text
	.globl	foo
foo:
	la	$2, lbar
	.org	0x2
	nop
	.space	0xffc
	.align	2
	.globl	bar
bar = .
lbar = .
	nop
