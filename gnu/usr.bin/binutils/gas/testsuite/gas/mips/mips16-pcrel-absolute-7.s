	.text

	.space	0x1000

	.set	bar, 0x12345678

	.ent	foo
	.set	mips16
foo:
	dla	$2, 0x12345678
	ld	$2, 0x12345678
	dla	$2, bar
	ld	$2, bar
	dla	$2, bar + 0x2468
	ld	$2, bar + 0x2468
	dla	$2, 0x2468ace0
	ld	$2, 0x2468ace0
	nop
	.set	nomips16
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
