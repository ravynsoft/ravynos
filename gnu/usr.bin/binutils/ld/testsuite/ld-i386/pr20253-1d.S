	.text
	.type	implementation1, @function
implementation1:
	movl	$1, %eax
	ret
	.size	implementation1, .-implementation1
	.type	implementation2, @function
implementation2:
	movl	$2, %eax
	ret
	.size	implementation2, .-implementation2
	.type	resolver2, @function
resolver2:
	call	__x86.get_pc_thunk.ax
	addl	$_GLOBAL_OFFSET_TABLE_, %eax
	leal	implementation2@GOTOFF(%eax), %eax
	ret
	.size	resolver2, .-resolver2
	.type	func2, @gnu_indirect_function
	.set	func2,resolver2
	.type	resolver1, @function
resolver1:
	call	__x86.get_pc_thunk.ax
	addl	$_GLOBAL_OFFSET_TABLE_, %eax
	leal	implementation1@GOTOFF(%eax), %eax
	ret
	.size	resolver1, .-resolver1
	.globl	func1
	.type	func1, @gnu_indirect_function
	.set	func1,resolver1
	.globl	get_func2
	.type	get_func2, @function
get_func2:
	call	__x86.get_pc_thunk.ax
	addl	$_GLOBAL_OFFSET_TABLE_, %eax
	movl	func2@GOT(%eax), %eax
	ret
	.size	get_func2, .-get_func2
	.globl	call_func2
	.type	call_func2, @function
call_func2:
	pushl	%ebx
	call	__x86.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	subl	$8, %esp
	call	*get_func2@GOT(%ebx)
	cmpl	func2@GOT(%ebx), %eax
	jne	.L10
	addl	$8, %esp
	movl	%ebx, %eax
	popl	%ebx
	jmp	*func2@GOT(%eax)
.L10:
	call	*abort@GOT(%ebx)
	.size	call_func2, .-call_func2
	.globl	func2_p
#ifdef CHECK_PLT
	.section	.rodata,"a",@progbits
#else
	.section	.data.rel,"aw",@progbits
#endif
	.align 4
	.type	func2_p, @object
	.size	func2_p, 4
func2_p:
	.long	func2
	.section	.text.__x86.get_pc_thunk.ax,"axG",@progbits,__x86.get_pc_thunk.ax,comdat
	.globl	__x86.get_pc_thunk.ax
	.hidden	__x86.get_pc_thunk.ax
	.type	__x86.get_pc_thunk.ax, @function
__x86.get_pc_thunk.ax:
	movl	(%esp), %eax
	ret
	.section	.text.__x86.get_pc_thunk.bx,"axG",@progbits,__x86.get_pc_thunk.bx,comdat
	.globl	__x86.get_pc_thunk.bx
	.hidden	__x86.get_pc_thunk.bx
	.type	__x86.get_pc_thunk.bx, @function
__x86.get_pc_thunk.bx:
	movl	(%esp), %ebx
	ret
	.section	.note.GNU-stack,"",@progbits
