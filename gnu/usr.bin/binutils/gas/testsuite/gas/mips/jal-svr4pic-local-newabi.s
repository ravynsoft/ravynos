	.abicalls
	.text

	.align	4, 0
	.globl	foo
	.ent	foo
foo:
	.frame	$sp, 16, $31
	.mask	0x90000000, -8
	.fmask	0x00000000, 0
	addiu	$sp,$sp,-16
	.cpsetup $25, 0, foo
	sd	$31, 8($sp)
	jal	bar
	ld	$31,8($sp)
	.cpreturn
	addiu	$sp,$sp,16
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
