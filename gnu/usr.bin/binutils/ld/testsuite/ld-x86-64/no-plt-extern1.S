	.text
	.p2align 4,,15
	.globl	get_func
	.type	get_func, @function
get_func:
	movq	func@GOTPCREL(%rip), %rax
	ret
	.size	get_func, .-get_func
	.p2align 4,,15
	.globl	call_func
	.type	call_func, @function
call_func:
	jmp	*func@GOTPCREL(%rip)
	.size	call_func, .-call_func
	.globl	func_p
	.section	.data.rel,"aw",@progbits
	.type	func_p, @object
func_p:
#ifdef __LP64__
	.align 8
	.size	func_p, 8
	.quad	func
#else
	.align 4
	.size	func_p, 4
	.long	func
#endif
	.section	.note.GNU-stack,"",@progbits
