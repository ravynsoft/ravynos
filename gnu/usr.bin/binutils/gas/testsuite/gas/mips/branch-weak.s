	.text
	.align	align, 0
	.globl	foo
	.ent	foo
foo:
	b	bar
	.end	foo

	.align	align, 0
	.globl	bar
	.weak	bar
	.ent	bar
bar:
	nop
	jr	$ra
	.end	bar

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align  align, 0
