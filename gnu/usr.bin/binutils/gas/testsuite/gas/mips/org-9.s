	.set	micromips
	.text
	.globl	foo
foo:
	addu	$4, $3, $2
	beqz	$2, lbar
	.org	0x4
	nop
	.space	0x7a
	.globl	bar
bar:
lbar:
	nop
