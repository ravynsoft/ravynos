	.text
	.globl	bar

	.space	0x1000

	.ent	bar
	.set	mips16
bar:
	nop
	.set	nomips16
	.end	bar

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
