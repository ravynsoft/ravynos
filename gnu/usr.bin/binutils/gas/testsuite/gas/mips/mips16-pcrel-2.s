	.text

bar:
	.space	0x10000

	.ent	foo
	.set	mips16
	.set	noreorder
	.set	nomacro
foo:
	la	$2, . - 1
	nop
	lw	$2, . - 1
	nop
	la	$2, . - 1 + 0x3fc
	nop
	lw	$2, . - 1 + 0x3fc
	nop
	la	$2, . - 1 + 0x400
	lw	$2, . - 1 + 0x400
	la	$2, . - 1 - 0x4
	lw	$2, . - 1 - 0x4
	la	$2, . - 1 + 0x7fff
	lw	$2, . - 1 + 0x7fff
	la	$2, . - 1 - 0x8000
	lw	$2, . - 1 - 0x8000
	la	$2, . - 1 + 0x8000
	lw	$2, . - 1 + 0x8000
	la	$2, . - 1 - 0x8001
	lw	$2, . - 1 - 0x8001
	nop
	.set	macro
	.set	reorder
	.set	nomips16
	.end	foo

baz:
	.align	8, 0
	.space	0xfe00

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
