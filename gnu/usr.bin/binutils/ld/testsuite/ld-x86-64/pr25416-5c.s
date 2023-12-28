	.text
	.p2align 4
	.globl	test1
	.type	test1, @function
test1:
	.cfi_startproc
	subq	$8, %rsp
	.cfi_def_cfa_offset 16
	lea	foo@TLSDESC(%rip), %rax
	call	*foo@TLSCALL(%rax)
	addl	%fs:0, %eax
	cmpl	%edi, (%eax)
	jne	.L5
	movl	%eax, %r8d
	addq	$8, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 8
	movq	%r8, %rax
	ret
.L5:
	.cfi_restore_state
	call	abort@PLT
	.cfi_endproc
	.size	test1, .-test1
	.p2align 4
	.globl	test2
	.type	test2, @function
test2:
	.cfi_startproc
	subq	$8, %rsp
	.cfi_def_cfa_offset 16
	lea	bar@TLSDESC(%rip), %rax
	call	*bar@TLSCALL(%rax)
	addl	%fs:0, %eax
	cmpl	%edi, (%eax)
	jne	.L9
	movl	%eax, %r8d
	addq	$8, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 8
	movq	%r8, %rax
	ret
.L9:
	.cfi_restore_state
	call	abort@PLT
	.cfi_endproc
	.size	test2, .-test2
	.section	.tdata,"awT",@progbits
	.align 4
	.hidden foo
	.globl foo
	.type	foo, @object
	.size	foo, 4
foo:
	.long	30
	.section	.note.GNU-stack,"",@progbits
