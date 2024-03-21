	.abicalls
	.text
	.set	noreorder
	.globl	foo
	.ent	foo
foo:
	.frame	$sp, 0, $31
	.mask	0x00000000, 0
	.fmask	0x00000000, 0
	.cpload	$25
	lwl	$2, %got(bar)($28)
	jr	$31
	 addiu	$2, $2, 4
	.end	foo
	.weak	bar
	.hidden	bar

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
