	.text
	.globl	main
	.type	main, @function
main:
	subq	$8, %rsp
	call   *foo@GOTPCREL(%rip)
	call   *bar@GOTPCREL(%rip)
	call   *plt@GOTPCREL(%rip)
	movq   foo@GOTPCREL(%rip), %rax
	call   *%rax
	movq   bar@GOTPCREL(%rip), %rcx
	call   *%rcx
	movq   plt@GOTPCREL(%rip), %rdx
	call   *%rdx
	xorl	%edi, %edi
	addq	$8, %rsp
	jmp	*myexit@GOTPCREL(%rip)
	.size	main, .-main
	.section	.note.GNU-stack
