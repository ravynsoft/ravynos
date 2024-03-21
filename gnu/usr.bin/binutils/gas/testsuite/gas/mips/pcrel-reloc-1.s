	.text
	.align  4, 0
	.globl	foo
	.ent	foo
	.set	noreorder
foo:
	b	bar
	b	bar
	b	bar
	b	bar
	.set	reorder
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align  4, 0
	.space  16

	.ent	bar
	.ifdef	setmips3
	.set	mips3
	.endif
	.ifdef	setmips64r6
	.set	mips64r6
	.endif
bar:
	jalr	$0, $ra
	.end	bar

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align  16, 0
	.space  16
