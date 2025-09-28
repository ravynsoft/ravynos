	.text
	.align	4
	.globl	foo
	.ent	foo
foo:
	ldpc	$2, bar0
	ldpc	$2, bar8
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
