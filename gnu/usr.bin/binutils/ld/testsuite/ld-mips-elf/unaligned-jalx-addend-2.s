	.text
	.align	4
	.globl	foo
	.ent	foo
foo:
	bal	bar0 + 4
	bal	bar1 - 2
	bal	bar2 - 4
	bal	bar3 + 2
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
