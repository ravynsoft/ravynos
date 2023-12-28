	.data
	.symver	foo, foo@FOO
	.symver	bar, bar@@FOO
	.globl	foo
	.type	foo, %object
foo:
	.byte 0
	.size	foo, .-foo
	.globl	bar
	.type	bar, %object
bar:
	.byte 0
	.size	bar, .-bar
