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
	leaq	implementation2(%rip), %rax
	ret
	.size	resolver2, .-resolver2
	.type	func2, @gnu_indirect_function
	.set	func2,resolver2
	.type	resolver1, @function
resolver1:
	leaq	implementation1(%rip), %rax
	ret
	.size	resolver1, .-resolver1
	.globl	func1
	.type	func1, @gnu_indirect_function
	.set	func1,resolver1
	.globl	get_func2
	.type	get_func2, @function
get_func2:
	movq	func2@GOTPCREL(%rip), %rax
	ret
	.size	get_func2, .-get_func2
	.globl	call_func2
	.type	call_func2, @function
call_func2:
	subq	$8, %rsp
	call	*get_func2@GOTPCREL(%rip)
	cmpq	func2@GOTPCREL(%rip), %rax
	jne	.L10
	addq	$8, %rsp
	jmp	*func2@GOTPCREL(%rip)
.L10:
	call	*abort@GOTPCREL(%rip)
	.size	call_func2, .-call_func2
	.globl	func2_p
#ifdef CHECK_PLT
	.section	.rodata,"a",@progbits
#else
	.section	.data.rel,"aw",@progbits
#endif
#ifdef __LP64__
	.align 8
	.size	func2_p, 8
#else
	.align 4
	.size	func2_p, 4
#endif
	.type	func2_p, @object
func2_p:
	.dc.a	func2
	.section	.note.GNU-stack,"",@progbits
