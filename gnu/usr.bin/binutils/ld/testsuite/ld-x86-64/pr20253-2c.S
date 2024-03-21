	.text
	.globl	get_func1
	.type	get_func1, @function
get_func1:
	movq	func1@GOTPCREL(%rip), %rax
	ret
	.size	get_func1, .-get_func1
	.globl	call_func1
	.type	call_func1, @function
call_func1:
	jmp	*func1@GOTPCREL(%rip)
	.size	call_func1, .-call_func1
	.globl	func1_p
#ifdef CHECK_PLT
	.section	.rodata,"a",@progbits
#else
	.section	.data.rel,"aw",@progbits
#endif
#ifdef __LP64__
	.align 8
	.size	func1_p, 8
#else
	.align 4
	.size	func1_p, 4
#endif
	.type	func1_p, @object
func1_p:
	.dc.a	func1
	.section	.note.GNU-stack,"",@progbits
