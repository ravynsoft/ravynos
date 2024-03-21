	.text
	.type	bar, @function
bar:
	ret
	.globl	foo
	.type	foo, @function
foo:
	ret
	.globl	_start
	.type	_start, @function
_start:
	call	*foo@GOT
	call	*bar@GOT
	jmp	*foo@GOT
	jmp	*bar@GOT
