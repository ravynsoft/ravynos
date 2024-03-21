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
	call	*foo@GOTPCREL(%rip)
	jmp	*bar@GOTPCREL(%rip)
	movq	$0, bar@GOTPCREL(%rip)
	cmpq	$0, foo@GOTPCREL(%rip)
	cmpq	foo@GOTPCREL(%rip), %rcx
	cmpq	bar@GOTPCREL(%rip), %rcx
