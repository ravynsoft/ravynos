	.abicalls
	.text

	.align	4, 0
	.globl	foo
	.ent	foo
foo:
	.frame	$sp, 32, $31
	.mask	0x80000000, -4
	.fmask	0x00000000, 0
	.set	noreorder
	.cpload	$25
	.set	reorder
	addiu	$sp, $sp, -32
	sw	$31, 28($sp)
	.cprestore 16
	jal	bar
	lw	$31, 28($sp)
	addiu	$sp, $sp, 32
	jr	$31
	.end	foo

	.align	4, 0
	.ent	bar
bar:
	.frame	$sp, 0, $31
	.mask	0x00000000, 0
	.fmask	0x00000000, 0
	jr	$31
	.end	bar

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
