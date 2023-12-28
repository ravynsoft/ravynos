	.data
	.globl	foo
	.type	foo, @object
foo:
	.text
	.globl	_start
	.type	_start, @function
_start:
	addl	foo@GOT, %ebx
