	.text
	.p2align 4
	.globl	test3
	.type	test3, @function
test3:
	.cfi_startproc
	subl	$8, %esp
	.cfi_def_cfa_offset 16
	lea	foo@TLSDESC(%rip), %eax
	call	*foo@TLSCALL(%eax)
	addl	%fs:0, %eax
	cmpl	%edi, (%eax)
	jne	.L5
	addl	$8, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 8
	ret
.L5:
	.cfi_restore_state
	call	abort@PLT
	.cfi_endproc
	.size	test3, .-test3
	.section	.note.GNU-stack,"",@progbits
