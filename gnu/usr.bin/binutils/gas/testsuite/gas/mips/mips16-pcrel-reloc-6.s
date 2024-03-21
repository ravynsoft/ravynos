	.text
	.globl	bar

	.space	0x1000

	.ent	foo
	.set	mips16
foo:
	dla	$2, bar
	ld	$2, bar
	nop
	.set	nomips16
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16

	.type	bar, @object
bar:
	.long	0
	.size	bar, . - bar

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
