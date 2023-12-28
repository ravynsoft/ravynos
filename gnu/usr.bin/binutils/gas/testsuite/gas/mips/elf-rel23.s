	.abicalls
	.globl	foo
	.ent	foo
foo:
	.cpsetup $4,$5,foo
	.end	foo

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
