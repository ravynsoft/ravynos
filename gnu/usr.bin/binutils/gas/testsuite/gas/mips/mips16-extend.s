	.set	mips16
	.set	noreorder
foo:
	extend	0
	addiu	$16, $29, 0
	extend	1
	addiu	$16, $29, 0
	extend	1445
	addiu	$16, $29, 0
	extend	2047
	addiu	$16, $29, 0
	extend	0x123
	addiu	$16, $29, 0
	extend	0x432
	addiu	$16, $29, 0
	extend	0x789
	addiu	$16, $29, 0
	extend	0x7ff
	addiu	$16, $29, 0

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
