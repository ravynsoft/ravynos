	.text
	.globl	foo
	.type	foo, @gnu_indirect_function
foo:
	ret
	.text
	.type	bar, @gnu_indirect_function
bar:
	ret
	.globl	_start
	.type	_start, @function
_start:
	call	*foo@GOT
	jmp	*bar@GOT
	movl	$0, bar@GOT
	cmpl	$0, foo@GOT
	movl	$bar@GOT, %ecx
