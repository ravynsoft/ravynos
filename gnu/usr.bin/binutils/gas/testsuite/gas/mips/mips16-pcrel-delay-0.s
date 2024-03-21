	.text

bar:
	.space	0x10000

	.ent	foo
	.set	mips16
	.set	noreorder
foo:
	jr	$4
	 la	$2, . - 3 + 0x3fc
	jr	$ra
	 lw	$2, . - 3 + 0x3fc
	jr	$4
	 la	$2, . - 3 + 0x7fff
	nop
	jr	$ra
	 lw	$2, . - 3 + 0x7fff
	nop
	jr	$4
	 la	$2, . - 3 + 0x8000
	nop
	jr	$ra
	 lw	$2, . - 3 + 0x8000
	nop
	.set	reorder
	.set	nomips16
	.end	foo

baz:
	.align	8, 0
	.space	0xfe00

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
