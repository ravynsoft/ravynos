	.text

	.space	0x1000

	.globl	foo
	.ent	foo
	.set	mips16
foo:
	b	bar
	bteqz	bar
	btnez	bar
	beqz	$2, bar
	bnez	$2, bar
	nop
	.set	nomips16
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16

	.set	bar, 0x1235
