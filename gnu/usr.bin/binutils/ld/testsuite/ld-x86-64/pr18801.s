	.text
	.type   selector, %function
foo:
	movl	$0, %eax
	ret
selector:
	movabs	$foo, %rax
	ret
	.type   selector, %gnu_indirect_function
	.globl	_start
_start:
	movabs	$selector, %rax
	call	*%rax
	ret
	.section	.note.GNU-stack,"",@progbits
