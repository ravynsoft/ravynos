	.text
	.type   selector, %function
foo:
	movl	$0, %eax
	ret
selector:
	mov	$foo, %eax
	ret
	.type   selector, %gnu_indirect_function
	.globl	_start
_start:
	mov	$selector, %eax
	call	*%eax
	ret
	.section	.note.GNU-stack,"",@progbits
