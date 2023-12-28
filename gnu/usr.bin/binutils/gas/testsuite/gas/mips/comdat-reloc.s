	.abicalls

	.section	.text.foo, "axG", @progbits, bar, comdat
	.align	2
	.ent	foo
	.type	foo, @function
foo:
	.frame	$sp, 0, $31
	.mask	0x00000000, 0
	.fmask	0x00000000, 0
	jr	$31
	.end	foo
	.size	foo, . - foo

	.section	.text.bar, "axG", @progbits, bar, comdat
	.align	2
	.globl	bar
	.ent	bar
	.type	bar, @function
bar:
	.frame	$sp, 0, $31
	.mask	0x00000000, 0
	.fmask	0x00000000, 0
	.set	noreorder
	.cpload	$25
	.set	reorder
	beqz	$4, 1f
	.set	noreorder
	lw	$2, %got(foo)($28)
0:
	jr	$31
	 addiu	$2, $2, %lo(foo)
1:
	b	0b
	 lw	$2, %got(foo)($28)
	.set	reorder
	.end	bar
	.size	bar, . - bar
