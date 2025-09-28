	.abicalls
	.set	noreorder

	.type	obj, @object
	.weak	obj
	.ifdef	prot
	.protected obj
	.endif
	.ifdef	hidn
	.hidden	obj
	.endif
	.ifdef	intr
	.internal obj
	.endif

	.section .text.foo, "ax", @progbits
	.globl	foo
	.ent	foo
foo:
	.frame	$sp, 0, $31
	.mask	0x00000000, 0
	.fmask	0x00000000, 0
	.cplocal $4
	.cpsetup $25, $0, foo
	lw	$2, %got_page(obj + 4)($4)
	lw	$3, %got_disp(obj)($4)
	addiu	$2, %got_ofst(obj + 4)
	jr	$31
	 addiu	$3, 4
	.end	foo

# Pad a little so that the microMIPS version aligns the same.
	.space	4

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16

	.section .text.bar, "ax", @progbits
	.ent	bar
bar:
	.frame	$sp, 0, $31
	.mask	0x00000000, 0
	.fmask	0x00000000, 0
	.cplocal $4
	.cpsetup $25, $0, bar
	lwl	$2, %got_page(obj + 4)($4)
	lwr	$3, %got_disp(obj)($4)
	addiu	$2, %got_ofst(obj + 4)
	jr	$31
	 addiu	$3, 4
	.end	bar

# Pad a little so that the microMIPS version aligns the same.
	.space	4

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
