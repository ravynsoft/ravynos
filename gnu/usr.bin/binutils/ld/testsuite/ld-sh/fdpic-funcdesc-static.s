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
	.globl	_start
	.type	_start,@function
_start:
	nop
