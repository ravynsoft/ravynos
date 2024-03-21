	.abicalls
	.set	noreorder

	.type	fun, @function
	.weak	fun
	.type	obj, @object
	.weak	obj
	.ifdef	prot
	.protected fun
	.protected obj
	.endif
	.ifdef	hidn
	.hidden	fun
	.hidden	obj
	.endif
	.ifdef	intr
	.internal fun
	.internal obj
	.endif

	.section .text.foo, "ax", @progbits
	.globl	foo
	.ent	foo
foo:
	.frame	$sp, 0, $31
	.mask	0x00000000, 0
	.fmask	0x00000000, 0
	li	$2, %hi(_gp_disp)
	addiu	$3, $pc, %lo(_gp_disp)
	sll	$2, 16
	addu	$2, $3
	lw	$4, %got(obj)($2)
	lw	$2, %call16(fun)($2)
	jr	$2
	 move	$25,$2
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16

	.section .text.bar, "ax", @progbits
	.ent	bar
bar:
	.frame	$sp, 0, $31
	.mask	0x00000000, 0
	.fmask	0x00000000, 0
	li	$2, %hi(_gp_disp)
	addiu	$3, $pc, %lo(_gp_disp)
	sll	$2, 16
	addu	$2, $3
	move	$4, $2
	addiu	$4, %got(obj)
	addiu	$2, %call16(fun)
	lw	$2, 0($2)
	lw	$4, 0($4)
	jr	$2
	 move	$25,$2
	.end	bar

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
