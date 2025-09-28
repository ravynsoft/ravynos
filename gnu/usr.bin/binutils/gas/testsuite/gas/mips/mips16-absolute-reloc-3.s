	.text

	.space	0x1000

	.globl	foo
	.ent	foo
	.set	mips16
foo:
	daddiu	$2, bar
	daddiu	$2, $3, bar
	daddiu	$2, $pc, bar
	daddiu	$sp, bar
	daddiu	$2, $sp, bar
	dsll	$2, $3, bar
	dsra	$2, bar
	dsrl	$2, bar
	ld	$2, bar($3)
	ld	$2, bar($pc)
	ld	$2, bar($sp)
	lwu	$2, bar($3)
	sd	$2, bar($3)
	sd	$2, bar($sp)
	sd	$ra, bar($sp)
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
