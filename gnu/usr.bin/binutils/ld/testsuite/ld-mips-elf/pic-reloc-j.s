	.text
	.globl	foo
	.ent	foo
foo:
	j	bar
	j	bar
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
