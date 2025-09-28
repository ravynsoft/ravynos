	.text
	.p2align 4
	.globl	get_bar
	.type	get_bar, @function
get_bar:
	.cfi_startproc
	call	__x86.get_pc_thunk.ax
	addl	$_GLOBAL_OFFSET_TABLE_, %eax
	movl	bar@GOT(%eax), %eax
	ret
	.cfi_endproc
	.size	get_bar, .-get_bar
	bar = 0xfffffff0
	.section	.text.__x86.get_pc_thunk.ax,"axG",@progbits,__x86.get_pc_thunk.ax,comdat
	.globl	__x86.get_pc_thunk.ax
	.hidden	__x86.get_pc_thunk.ax
	.type	__x86.get_pc_thunk.ax, @function
__x86.get_pc_thunk.ax:
	.cfi_startproc
	movl	(%esp), %eax
	ret
	.cfi_endproc
	.section	.note.GNU-stack,"",@progbits
