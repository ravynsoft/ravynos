	.text
	.globl	main
	.type	main,@function
main:
	pushq	%rax
        movslq  bar(%rip), %rax
        leaq    bar(%rip), %rdi
	addq	%rax, %rdi

	callq	foo
	xorl	%eax, %eax
	popq	%rcx
	retq

	.data
	.p2align 2
bar:
	.long	test - .

	.section	.note.GNU-stack,"",@progbits
