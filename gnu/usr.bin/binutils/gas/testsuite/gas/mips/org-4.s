	.text
	.globl	foo
foo:
	beqz	$2, lbar
	.org	0xc
	nop
	.space	0xffff0
	.globl	bar
bar:
lbar:
	nop
