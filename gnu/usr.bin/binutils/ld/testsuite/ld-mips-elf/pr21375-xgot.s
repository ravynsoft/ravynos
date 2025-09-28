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
	lui	$4, %got_hi(obj)
	lui	$25, %call_hi(fun)
	addu	$4, $28
	addu	$25, $28
	lw	$25, %call_lo(fun)($25)
	lw	$4, %got_lo(obj)($4)
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
	lui	$4, %got_hi(obj)
	lui	$25, %call_hi(fun)
	addu	$4, $28
	addu	$25, $28
	lwl	$25, %call_lo(fun)($25)
	lwr	$4, %got_lo(obj)($4)
	jr	$25
	 addiu	$4, 4
	.end	bar

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
