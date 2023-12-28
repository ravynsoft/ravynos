	.text
	.globl	foo
	.type	foo, @function
foo:
	ret
	.globl	_start
	.type	_start, @function
_start:
	jmp	*foo@GOT
