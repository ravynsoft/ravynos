	.text
	.p2align 4,,15
	.globl	main
	.type	main, @function
main:
	subq	$8, %rsp
	call	foo
	xorl	%eax, %eax
	addq	$8, %rsp
	ret
	.size	main, .-main
	.section	.note.GNU-stack,"",@progbits
