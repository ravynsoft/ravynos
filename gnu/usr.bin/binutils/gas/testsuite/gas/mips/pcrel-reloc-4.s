	.text

	.ifdef	reverse
	.ent	baz
baz:
	jalr	$0, $ra
	.end	baz

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align  16, 0
	.space  16

	.ent	bar
bar:
	jalr	$0, $ra
	.end	bar

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align  4, 0
	.space  16
	.endif

	.align  4, 0
	.globl	foo
	.ent	foo
	.set	noreorder
foo:
	b	bar
	b	bar
	b	bar
	b	bar
	.set	mips64r6
	bc	bar
	bc	bar
	bc	bar
	bc	bar
	beqzc	$2, bar
	beqzc	$2, bar
	beqzc	$2, bar
	beqzc	$2, bar
	lwpc	$2, bar
	lwpc	$2, bar
	lwpc	$2, bar
	lwpc	$2, bar
	ldpc	$2, bar
	ldpc	$2, bar
	ldpc	$2, bar
	ldpc	$2, bar
	auipc	$2, %pcrel_hi(baz)
	addiu	$2, %pcrel_lo(baz + 4)
	auipc	$2, %pcrel_hi(baz)
	addiu	$2, %pcrel_lo(baz + 4)
	auipc	$2, %pcrel_hi(baz)
	addiu	$2, %pcrel_lo(baz + 4)
	auipc	$2, %pcrel_hi(baz)
	addiu	$2, %pcrel_lo(baz + 4)
	.set	mips0
	.set	reorder
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align  4, 0
	.space  16

	.ifndef	reverse
	.ent	bar
bar:
	jalr	$0, $ra
	.end	bar

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align  16, 0
	.space  16

	.ent	baz
baz:
	jalr	$0, $ra
	.end	baz

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align  4, 0
	.space  16
	.endif
