	.globl	foo
	.csect	foo[RW]
foo:
	.long	.puts
	.long	foobar
