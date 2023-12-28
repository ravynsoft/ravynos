	.set	mips16
foo:
	jalx.t	bar
	jal.t	baz

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
