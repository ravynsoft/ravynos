	.text
	.p2align 4
	.globl	test3
	.type	test3, @function
test3:
	.cfi_startproc
	movl	%fs:0, %eax
	addq	foo@gottpoff(%rip), %rax
	cmpl	%edi, (%eax)
	jne	.L7
	movl	%eax, %eax
	ret
.L7:
	pushq	%rax
	.cfi_def_cfa_offset 16
	call	abort@PLT
	.cfi_endproc
	.size	test3, .-test3
	.section	.note.GNU-stack,"",@progbits
