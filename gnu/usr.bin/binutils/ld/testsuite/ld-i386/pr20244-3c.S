	.text
	.p2align 4,,15
	.globl	get_func1
	.type	get_func1, @function
get_func1:
	movl	func1@GOT, %eax
	ret
	.size	get_func1, .-get_func1
	.p2align 4,,15
	.globl	call_func1
	.type	call_func1, @function
call_func1:
	jmp	*func1@GOT
	.size	call_func1, .-call_func1
	.section	.note.GNU-stack,"",@progbits
