	.set	nomips16
	.ent	foo
foo:
	lui	$4,%highest(bar)
	dsll	$4,$4,16
	daddiu	$4,$4,%higher(bar)
	dsll	$4,$4,16
	daddiu	$4,$4,%hi(bar)
	dsll	$4,$4,16
	lw	$4,%lo(bar)($4)
	.end	foo
	.eqv	bar,0x123456789abcdef0
