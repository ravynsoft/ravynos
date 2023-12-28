	.text

bar:
	.space	0x10000

	.ent	foo
	.set	mips16
	.set	noreorder
foo:
	jal	bat
	 la	$2, . - 5 + 0x3fc
	nop
	jalx	bax
	 lw	$2, . - 5 + 0x3fc
	nop
	jal	bat
	 la	$2, . - 5 + 0x7fff
	jalx	bax
	 lw	$2, . - 5 + 0x7fff
	jal	bat
	 la	$2, . - 5 + 0x8000
	jalx	bax
	 lw	$2, . - 5 + 0x8000
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
