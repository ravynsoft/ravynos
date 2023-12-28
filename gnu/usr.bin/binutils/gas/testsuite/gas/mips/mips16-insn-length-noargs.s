	.set	mips16
foo:
	entry.t		# comment
	entry.t
	exit.t		# comment
	exit.t
	nop.t		# comment
	nop.t

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
