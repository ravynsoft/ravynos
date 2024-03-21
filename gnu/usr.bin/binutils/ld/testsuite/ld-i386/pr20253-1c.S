	.text
	.globl	get_func1
	.type	get_func1, @function
get_func1:
	call	__x86.get_pc_thunk.ax
	addl	$_GLOBAL_OFFSET_TABLE_, %eax
	movl	func1@GOT(%eax), %eax
	ret
	.size	get_func1, .-get_func1
	.globl	call_func1
	.type	call_func1, @function
call_func1:
	call	__x86.get_pc_thunk.ax
	addl	$_GLOBAL_OFFSET_TABLE_, %eax
	jmp	*func1@GOT(%eax)
	.size	call_func1, .-call_func1
	.globl	func1_p
#ifdef CHECK_PLT
	.section	.rodata,"a",@progbits
#else
	.section	.data.rel,"aw",@progbits
#endif
	.align 4
	.type	func1_p, @object
	.size	func1_p, 4
func1_p:
	.long	func1
	.section	.text.__x86.get_pc_thunk.ax,"axG",@progbits,__x86.get_pc_thunk.ax,comdat
	.globl	__x86.get_pc_thunk.ax
	.hidden	__x86.get_pc_thunk.ax
	.type	__x86.get_pc_thunk.ax, @function
__x86.get_pc_thunk.ax:
	movl	(%esp), %eax
	ret
	.section	.note.GNU-stack,"",@progbits
