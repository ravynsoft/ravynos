	.text
	.align	4
	.globl	foo
	.ent	foo
foo:
	bal	bar0 + 1
	bal	bar0 + 2
	bal	bar0 + 3
	bal	bar1 + 1
	bal	bar1 + 3
	bal	bar1 + 4
	bal	bar2 - 1
	bal	bar2 - 2
	bal	bar2 - 3
	bal	bar3 - 1
	bal	bar3 - 3
	bal	bar3 - 4
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
