	.set	mips16
	.text
	.globl	foo
foo:
	la	$2, lbar
	.org	0x4
	nop
	.space	0xffa
	.align	2
	.globl	bar
bar = .
lbar = .
	nop
