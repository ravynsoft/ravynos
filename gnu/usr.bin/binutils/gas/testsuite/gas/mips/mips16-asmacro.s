	.set	mips16
foo:
	asmacro	0, 0, 0, 0, 0, 0
	asmacro	0, 1, 2, 3, 4, 5
	asmacro	0, 31, 0, 7, 0, 7
	asmacro	5, 4, 3, 2, 1, 0
	asmacro	7, 0, 7, 0, 31, 0
	asmacro	7, 31, 7, 7, 31, 7

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
