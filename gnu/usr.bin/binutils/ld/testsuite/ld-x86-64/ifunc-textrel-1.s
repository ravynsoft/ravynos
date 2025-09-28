	.text
	.type   selector, %function
foo:
	movl	$0, %eax
	ret
selector:
.ifdef __x86_64__
	leaq	foo(%rip), %rax
.else
	leal	foo@GOTOFF(%eax), %eax
.endif
	ret
	.type   selector, %gnu_indirect_function
	.globl	_start
_start:
.ifdef __x86_64__
	movabs	ptr, %rax
	call	*%rax
.else
	mov	ptr, %eax
	call	*%eax
.endif
	ret
	.data
	.type	ptr, @object
ptr:
	.dc.a	selector
	.section	.note.GNU-stack,"",@progbits
