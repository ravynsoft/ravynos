	.abicalls
	.text
	.globl	foo
	.ent	foo
foo:
	la	$2, bar
	.option	pic0		# Switch off!
	la	$2, bar
	.option	pic2		# Switch on!
	la	$2, bar
	jr	$ra
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
