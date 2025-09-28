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
	movl	$implementation2, %eax
	ret
	.size	resolver2, .-resolver2
	.type	func2, @gnu_indirect_function
	.set	func2,resolver2
	.type	resolver1, @function
resolver1:
	movl	$implementation1, %eax
	ret
	.size	resolver1, .-resolver1
	.globl	func1
	.type	func1, @gnu_indirect_function
	.set	func1,resolver1
	.globl	get_func2
	.type	get_func2, @function
get_func2:
	movl	func2@GOT, %eax
	ret
	.size	get_func2, .-get_func2
	.globl	call_func2
	.type	call_func2, @function
call_func2:
	jmp	*func2@GOT
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
	.section	.note.GNU-stack,"",@progbits
