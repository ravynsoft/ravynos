	.set	mips16
foo:
	jalx.e	bar
	jal.e	baz

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
