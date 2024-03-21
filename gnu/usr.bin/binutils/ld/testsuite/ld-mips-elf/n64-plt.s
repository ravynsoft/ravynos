	.abicalls
	.option	pic0
	.text

	.globl	foo
	.ent	foo
foo:
	j	bar
	.end	foo
