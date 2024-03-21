	.text
	.abicalls
	.align  4, 0
	.globl	foo
	.ent	foo
	.set	noreorder
	.set	mips64r6
foo:
	b	(. + 4) - (0x20000 + offset)
	b	(. + 4) + (0x1fffc + offset)
	bc	(. + 4) - (0x8000000 + offset)
	bc	(. + 4) + (0x7fffffc + offset)
	beqzc	$2, (. + 4) - (0x400000 + offset)
	beqzc	$2, (. + 4) + (0x3ffffc + offset)
	lwpc	$2, . - (0x100000 + offset)
	lwpc	$2, . + (0xffffc + offset)
	ldpc	$2, . - (0x100000 + (offset << 1))
	ldpc	$2, (. - 4) + (0xffff8 + (offset << 1))
	.set	mips0
	.set	reorder
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align  4, 0
	.space  16
