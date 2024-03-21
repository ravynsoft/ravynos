	.text
	.p2align 4,,15
	.globl	foo
	.type	foo, @function
foo:
.LFB0:
	.cfi_startproc
	subq	$8, %rsp
	.cfi_def_cfa_offset 16
	call	bar2@PLT
	addq	$8, %rsp
	.cfi_def_cfa_offset 8
	jmp	bar1@PLT
	.cfi_endproc
.LFE0:
	.size	foo, .-foo
	.section	.note.GNU-stack,"",@progbits
