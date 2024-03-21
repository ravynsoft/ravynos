	.set	mips16
	.text
	.globl	foo
foo:
	la	$2, lbar
	.org	0x1000
	.align	2
	.globl	bar
bar = .
lbar = .
	nop
