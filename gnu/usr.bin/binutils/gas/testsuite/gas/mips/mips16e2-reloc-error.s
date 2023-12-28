	.text

	.set	bar, 4

	.ent	foo
	.set	mips16
foo:
	li	$2, %hi(bar)
	sll	$2, $2, 16

	ext	$3, $2, %lo(bar), 16
	ext	$3, $2, 16, %lo(bar)
	ins	$3, $2, %lo(bar), 16
	ins	$3, $2, 16, %lo(bar)
	ins	$2, $0, %lo(bar), 16
	ins	$2, $0, 16, %lo(bar)

	sync	%lo(bar)

	ll	$3, %lo(bar)($2)
	lwl	$3, %lo(bar)($2)
	lwr	$3, %lo(bar)($2)
	sc	$3, %lo(bar)($2)
	swl	$3, %lo(bar)($2)
	swr	$3, %lo(bar)($2)

	cache	3, %lo(bar)($2)
	pref	3, %lo(bar)($2)

	mfc0	$3, $2, %lo(bar)
	mtc0	$3, $2, %lo(bar)

	nop
	.set	nomips16
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.space	16
	.align	4, 0
