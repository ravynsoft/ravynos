	.set	mips1
	.set	mips16
foo:
	extend	0x123
	jal	bar

	extend	0x123
	jalx	baz

	extend	0x123
	jr	$3

	extend	0x123
	jr	$31

	extend	0x123
	jalr	$3

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
