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
	.cpload	$25
	lw	$25, %call16(fun)($28)
	lw	$4, %got(obj)($28)
	jr	$25
	 addiu	$4, 4
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
	.cpload	$25
	lwl	$25, %call16(fun)($28)
	lwr	$4, %got(obj)($28)
	jr	$25
	 addiu	$4, 4
	.end	bar

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
