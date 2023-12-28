	.text
	.align	4
	.globl	foo
	.ent	foo
foo:
	bal	bar0
	bal	bar2
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
