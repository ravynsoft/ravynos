	.globl	foo
	.weak	foo
	.type	foo,%object
	.size	foo,1

	.globl	bar
	.type	bar,%object
	.size	bar,1

	.data
foo:
bar:
	.byte	1
