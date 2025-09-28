	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"PASS"
	.text
	.globl	main
	.type	main, @function
main:
	movq	foo@GOTPCREL(%rip), %rax
	cmpl	$0, (%rax)
	jne	.L4
	subq	$8, %rsp
	call	foo_p@PLT
	cmpq	foo@GOTPCREL(%rip), %rax
	jne	.L2
	movq	bar@GOTPCREL(%rip), %rax
	cmpl	$-1, (%rax)
	jne	.L2
	call	bar_p@PLT
	cmpq	bar@GOTPCREL(%rip), %rax
	jne	.L2
	leaq	.LC0(%rip), %rdi
	call	puts@PLT
.L2:
	movl	$0, %eax
	addq	$8, %rsp
	ret
.L4:
	movl	$0, %eax
	ret
	.size	main, .-main
	.comm	bar,8,4
	.comm	foo,4,4
	.section	.note.GNU-stack,"",@progbits
