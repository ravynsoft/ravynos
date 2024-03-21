	.text

	.space	0x1000

	.globl	foo
	.ent	foo
	.set	mips16
foo:
	daddiu	$16, 1f - .
	daddiu	$4, $29, 0x1234
1:
	nop
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
