	.text

	.space	0x1000

	.ent	foo
	.set	mips16
foo:
	b	bar + 4 + (0x7fff << 1)
	bteqz	bar + 4 + (0x7fff << 1)
	btnez	bar + 4 + (0x7fff << 1)
	beqz	$2, bar + 4 + (0x7fff << 1)
	bnez	$2, bar + 4 + (0x7fff << 1)
	b	bar + 4 - (0x8000 << 1)
	bteqz	bar + 4 - (0x8000 << 1)
	btnez	bar + 4 - (0x8000 << 1)
	beqz	$2, bar + 4 - (0x8000 << 1)
	bnez	$2, bar + 4 - (0x8000 << 1)
	nop
	.set	nomips16
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
