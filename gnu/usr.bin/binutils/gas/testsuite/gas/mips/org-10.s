	.text
	.globl	foo
foo:
	beqz	$2, lbar
	.org	0x10
	nop
	.space	0xfffec
	.globl	bar
bar:
lbar:
	nop
