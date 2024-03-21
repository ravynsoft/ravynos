	.module	mips3
	.set	mips16
foo:
	sw	$2, 0x10($29)
	jr	$3

	sw	$31, 0x18($29)
	jr	$3

	sd	$2, 0x20($29)
	jr	$3

	sd	$31, 0x28($29)
	jr	$3

	lw	$2, 0x30($29)
	jr	$3

	ld	$2, 0x38($29)
	jr	$3

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
