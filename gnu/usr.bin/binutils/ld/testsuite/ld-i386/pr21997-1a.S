	.text
	.p2align 4,,15
	.globl	get_protected
	.type	get_protected, @function
get_protected:
	call	__x86.get_pc_thunk.ax
	addl	$_GLOBAL_OFFSET_TABLE_, %eax
	movl	protected@GOTOFF(%eax), %eax
	ret
	.size	get_protected, .-get_protected
	.p2align 4,,15
	.globl	get_protected_p
	.type	get_protected_p, @function
get_protected_p:
	call	__x86.get_pc_thunk.ax
	addl	$_GLOBAL_OFFSET_TABLE_, %eax
	leal	protected@GOTOFF(%eax), %eax
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
	.section	.text.__x86.get_pc_thunk.ax,"axG",@progbits,__x86.get_pc_thunk.ax,comdat
	.globl	__x86.get_pc_thunk.ax
	.hidden	__x86.get_pc_thunk.ax
	.type	__x86.get_pc_thunk.ax, @function
__x86.get_pc_thunk.ax:
	movl	(%esp), %eax
	ret
	.section	.note.GNU-stack,"",@progbits
