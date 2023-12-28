	.text

	.space	0x1000

	.globl	foo
	.ent	foo
	.set	mips16
foo:
	b	0x1235
	bteqz	0x1235
	btnez	0x1235
	beqz	$2, 0x1235
	bnez	$2, 0x1235
	nop
	.set	nomips16
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
