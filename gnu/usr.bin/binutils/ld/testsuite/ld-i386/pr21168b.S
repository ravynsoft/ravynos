	.text
	.globl	bar
	.type	bar, @function
bar:
	call	__x86.get_pc_thunk.ax
	addl	$_GLOBAL_OFFSET_TABLE_, %eax
	lea	foo@GOT, %ecx
	mov	(%eax,%ecx,1), %eax
	ret
	.globl	bar_ifunc
	.type	bar_ifunc, @function
bar_ifunc:
	call	__x86.get_pc_thunk.ax
	addl	$_GLOBAL_OFFSET_TABLE_, %eax
	lea	ifunc@GOT, %ecx
	mov	(%eax,%ecx,1), %eax
	ret
	.type ifunc, @gnu_indirect_function
ifunc:
	mov	$0xbadbeef, %eax
	ret
	.section	.text.__x86.get_pc_thunk.ax,"axG",@progbits,__x86.get_pc_thunk.ax,comdat
	.globl	__x86.get_pc_thunk.ax
	.hidden	__x86.get_pc_thunk.ax
	.type	__x86.get_pc_thunk.ax, @function
__x86.get_pc_thunk.ax:
	movl	(%esp), %eax
	ret
	.section	.note.GNU-stack,"",@progbits
