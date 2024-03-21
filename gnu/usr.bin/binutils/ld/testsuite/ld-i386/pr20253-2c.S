	.text
	.globl	get_func1
	.type	get_func1, @function
get_func1:
	movl	func1@GOT, %eax
	ret
	.size	get_func1, .-get_func1
	.globl	call_func1
	.type	call_func1, @function
call_func1:
	jmp	*func1@GOT
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
	.section	.note.GNU-stack,"",@progbits
