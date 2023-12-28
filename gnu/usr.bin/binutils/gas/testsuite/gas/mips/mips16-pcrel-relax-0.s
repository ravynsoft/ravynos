	.text

	.space	0x1000

	.ent	foo
	.set	mips16
	.set	noreorder
foo:
	la	$2, 0f
	jr	$ra
	 nop
	.ifdef	align
	.align	2
	.endif
0:
	.short	0
	.set	reorder
	.set	nomips16
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
