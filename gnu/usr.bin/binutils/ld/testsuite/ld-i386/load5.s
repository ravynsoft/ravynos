	.text
	.type	foo, @function
foo:
	ret
	.globl	_start
	.type	_start, @function
_start:
	movl	foo@GOT, %eax
