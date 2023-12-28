	.text
	.p2align 4
	.globl	_start
	.type	_start, @function
_start:
	.cfi_startproc
	subq	$8, %rsp
	.cfi_def_cfa_offset 16
	leaq	foo@TLSDESC(%rip), %r9
	movq	%r9, %rax
	call	*foo@TLSCALL(%rax)
	addq	%fs:0, %rax
	addq	$8, %rsp
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
	.size	_start, .-_start
	.section	.tdata,"awT",@progbits
	.align 4
	.type	foo, @object
	.size	foo, 4
foo:
	.long	30
	.section	.note.GNU-stack,"",@progbits
