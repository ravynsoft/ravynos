	.module	mips64
	.set	mips16
	.set	noreorder
	.set	noautoextend

	.align	12, 0
foo0:
	nop
	nop
	addiu	$2, $pc, 0x3fc
	nop
	nop
	nop
	lw	$3, 0x3fc($pc)
	nop
	nop
	nop
	daddiu	$4, $pc, 0x7c
	nop
	nop
	nop
	nop
	nop
	ld	$5, 0xf8($pc)

	.align	12, 0
foo1:
	jal	bar0
	 addiu	$2, $pc, 0x3fc
	nop
	jal	bar0
	 lw	$3, 0x3fc($pc)
	nop
	jal	bar0
	 daddiu	$4, $pc, 0x7c
	nop
	nop
	nop
	jal	bar0
	 ld	$5, 0xf8($pc)

	.align	12, 0
foo2:
	jalx	bar1
	 addiu	$2, $pc, 0x3fc
	nop
	jalx	bar1
	 lw	$3, 0x3fc($pc)
	nop
	jalx	bar1
	 daddiu	$4, $pc, 0x7c
	nop
	nop
	nop
	jalx	bar1
	 ld	$5, 0xf8($pc)

	.align	12, 0
foo3:
	nop
	jr	$16
	 addiu	$2, $pc, 0x3fc
	nop
	nop
	jr	$16
	 lw	$3, 0x3fc($pc)
	nop
	nop
	jr	$16
	 daddiu	$4, $pc, 0x7c
	nop
	nop
	nop
	nop
	jr	$16
	 ld	$5, 0xf8($pc)

	.align	12, 0
foo4:
	nop
	jr	$31
	 addiu	$2, $pc, 0x3fc
	nop
	nop
	jr	$31
	 lw	$3, 0x3fc($pc)
	nop
	nop
	jr	$31
	 daddiu	$4, $pc, 0x7c
	nop
	nop
	nop
	nop
	jr	$31
	 ld	$5, 0xf8($pc)

	.align	12, 0
foo5:
	nop
	jalr	$16
	 addiu	$2, $pc, 0x3fc
	nop
	nop
	jalr	$16
	 lw	$3, 0x3fc($pc)
	nop
	nop
	jalr	$16
	 daddiu	$4, $pc, 0x7c
	nop
	nop
	nop
	nop
	jalr	$16
	 ld	$5, 0xf8($pc)

	.align	12, 0
foo6:
	nop
	.half	0xe860
	addiu	$2, $pc, 0x3fc
	nop
	nop
	.half	0xe860
	lw	$3, 0x3fc($pc)
	nop
	nop
	.half	0xe860
	daddiu	$4, $pc, 0x7c
	nop
	nop
	nop
	nop
	.half	0xe860
	ld	$5, 0xf8($pc)

	.align	12, 0
foo7:
	nop
	jrc	$16
	addiu	$2, $pc, 0x3fc
	nop
	nop
	jrc	$16
	lw	$3, 0x3fc($pc)
	nop
	nop
	jrc	$16
	daddiu	$4, $pc, 0x7c
	nop
	nop
	nop
	nop
	jrc	$16
	ld	$5, 0xf8($pc)

	.align	12, 0
foo8:
	nop
	jrc	$31
	addiu	$2, $pc, 0x3fc
	nop
	nop
	jrc	$31
	lw	$3, 0x3fc($pc)
	nop
	nop
	jrc	$31
	daddiu	$4, $pc, 0x7c
	nop
	nop
	nop
	nop
	jrc	$31
	ld	$5, 0xf8($pc)

	.align	12, 0
foo9:
	nop
	jalrc	$16
	addiu	$2, $pc, 0x3fc
	nop
	nop
	jalrc	$16
	lw	$3, 0x3fc($pc)
	nop
	nop
	jalrc	$16
	daddiu	$4, $pc, 0x7c
	nop
	nop
	nop
	nop
	jalrc	$16
	ld	$5, 0xf8($pc)

	.align	12, 0
fooa:
	nop
	.half	0xe960
	addiu	$2, $pc, 0x3fc
	nop
	nop
	.half	0xe960
	lw	$3, 0x3fc($pc)
	nop
	nop
	.half	0xe960
	daddiu	$4, $pc, 0x7c
	nop
	nop
	nop
	nop
	.half	0xe960
	ld	$5, 0xf8($pc)

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	12, 0
