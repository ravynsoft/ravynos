	.set	micromips
	.text
	.globl	foo
foo:
	addu	$4, $3, $2
	beqz	$2, lbar
	.org	0x6
	nop
	.space	0xff8
	.globl	bar
bar:
lbar:
	nop
