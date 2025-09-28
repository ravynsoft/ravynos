	.module mips64r3
	.module	micromips
foo:
	.insn
	.short	0x7f6e, 0x5d4c

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
