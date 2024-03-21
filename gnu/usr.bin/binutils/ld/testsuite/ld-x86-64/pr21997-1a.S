	.text
	.p2align 4,,15
	.globl	get_protected
	.type	get_protected, @function
get_protected:
	movl	protected(%rip), %eax
	ret
	.size	get_protected, .-get_protected
	.p2align 4,,15
	.globl	get_protected_p
	.type	get_protected_p, @function
get_protected_p:
	leaq	protected(%rip), %rax
	ret
	.size	get_protected_p, .-get_protected_p
	.protected	protected
	.globl	protected
	.data
	.align 4
	.type	protected, @object
	.size	protected, 4
protected:
	.long	1
	.section	.note.GNU-stack,"",@progbits
