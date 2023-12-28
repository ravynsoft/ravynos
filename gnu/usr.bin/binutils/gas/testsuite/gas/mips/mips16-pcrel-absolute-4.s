	.text

	.space	0x1000

	.ent	foo
	.set	mips16
foo:
	la	$2, 0x12345678
	lw	$2, 0x12345678
	la	$2, bar
	lw	$2, bar
	la	$2, bar + 0x2468
	lw	$2, bar + 0x2468
	la	$2, 0x2468ace0
	lw	$2, 0x2468ace0
	nop
	.set	nomips16
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16

	.set	bar, 0x12345678
