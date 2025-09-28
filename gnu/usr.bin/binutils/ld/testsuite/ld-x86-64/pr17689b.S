	.text
	.globl	main
	.type	main, @function
main:
	subq	$8, %rsp
	movq	bar_alias(%rip), %rdi
	call	foo@PLT
	movq	ptr(%rip), %rax
	movq	(%rax), %rdi
	call	foo@PLT
	xorl	%eax, %eax
	addq	$8, %rsp
	ret
	.size	main, .-main
	.globl	ptr
	.section	.data.rel,"aw",@progbits
	.align 8
	.type	ptr, @object
	.size	ptr, 8
ptr:
	.quad	bar_alias
	.section	.note.GNU-stack,"",@progbits
