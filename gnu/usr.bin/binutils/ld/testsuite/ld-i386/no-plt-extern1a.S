	.text
	.p2align 4,,15
	.globl	get_func
	.type	get_func, @function
get_func:
	call	__x86.get_pc_thunk.ax
	addl	$_GLOBAL_OFFSET_TABLE_, %eax
	movl	func@GOT(%eax), %eax
	ret
	.size	get_func, .-get_func
	.p2align 4,,15
	.globl	call_func
	.type	call_func, @function
call_func:
	call	__x86.get_pc_thunk.ax
	addl	$_GLOBAL_OFFSET_TABLE_, %eax
	jmp	*func@GOT(%eax)
	.size	call_func, .-call_func
	.globl	func_p
	.section	.data.rel,"aw",@progbits
	.align 4
	.type	func_p, @object
	.size	func_p, 4
func_p:
	.long	func
	.section	.text.__x86.get_pc_thunk.ax,"axG",@progbits,__x86.get_pc_thunk.ax,comdat
	.globl	__x86.get_pc_thunk.ax
	.hidden	__x86.get_pc_thunk.ax
	.type	__x86.get_pc_thunk.ax, @function
__x86.get_pc_thunk.ax:
	movl	(%esp), %eax
	ret
	.section	.note.GNU-stack,"",@progbits
