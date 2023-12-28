	.text
	.globl	foo
foo:
	beqz	$2, lbar
	.org	0x8
	nop
	.space	0xfff4
	.globl	bar
bar:
lbar:
	nop
