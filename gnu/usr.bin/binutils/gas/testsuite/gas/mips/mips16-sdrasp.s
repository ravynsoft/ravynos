	.set	mips16
foo:
	sd	$31, 0($29)

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
