	.text

	.space	0x1000

	.ent	foo
	.set	mips16
foo:
	la	$2, bar
	lw	$2, bar
	nop
	.set	nomips16
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
