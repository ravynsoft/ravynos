	.data
	.globl foo
foo:
	.long -1
	.text
	.globl	_start
	.type	_start, @function
_start:
	pushl	foo@GOT(%ebx)
