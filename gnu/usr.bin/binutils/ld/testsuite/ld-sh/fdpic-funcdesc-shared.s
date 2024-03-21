	.data
	.globl	bar
	.type	bar,@object
	.size	bar,4
bar:
	.long	foo@FUNCDESC
	.text
	.type	foo,@function
foo:
	nop
