	.text
	.align	4
	.globl	foo
	.ent	foo
foo:
	lwpc	$2, bar0
	lwpc	$2, bar4
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
