	.text
	.align	4
	.globl	foo
	.ent	foo
foo:
	jal	bar0 + 1
	jal	bar0 + 2
	jal	bar0 + 3
	jal	bar1 + 1
	jal	bar1 + 3
	jal	bar1 + 4
	jal	bar2 - 1
	jal	bar2 - 2
	jal	bar2 - 3
	jal	bar3 - 1
	jal	bar3 - 3
	jal	bar3 - 4
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
