	.text
	.globl	foo
	.ent	foo
foo:
	nal
	jr	$ra
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
