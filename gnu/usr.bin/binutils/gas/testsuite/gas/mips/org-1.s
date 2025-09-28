	.text
	.globl	foo
foo:
	beqz	$2, lbar
	.org	0x100000
	.globl	bar
bar:
lbar:
	nop
