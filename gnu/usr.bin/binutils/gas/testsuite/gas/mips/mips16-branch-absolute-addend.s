	.text

	.space	0x1000

	.globl	foo
	.ent	foo
	.set	mips16
foo:
	b	bar + 0x1234
	bteqz	bar + 0x1234
	btnez	bar + 0x1234
	beqz	$2, bar + 0x1234
	bnez	$2, bar + 0x1234
	nop
	.set	nomips16
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16

	.set	bar, 0x12345679
