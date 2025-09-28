	.text

	.space	0x10000

	.ent	foo
	.set	mips16
	.set	noautoextend
foo:
	la.e	$2, . - 1 + 0x7fff
	lw.e	$2, . - 1 + 0x7fff
	la.e	$2, . - 1 - 0x8000
	lw.e	$2, . - 1 - 0x8000
	la.e	$2, . - 1 + 0x8000
	lw.e	$2, . - 1 + 0x8000
	la.e	$2, . - 1 - 0x8001
	lw.e	$2, . - 1 - 0x8001
	la.e	$2, bar
	lw.e	$2, bar
	nop
	.set	autoextend
	.set	nomips16
	.end	foo

	.align	8, 0
	.space	0xfe00

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
