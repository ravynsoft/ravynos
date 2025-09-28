	.text
	.set	mips16
	.ent	foo
foo:
	nop
	la	$2, bar
	lw	$2, bar
	dla	$2, bar
	ld	$2, bar
	.set	nomips16
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16

	.type	bar, @object
bar:
	.long	0
	.size	bar, . - bar
