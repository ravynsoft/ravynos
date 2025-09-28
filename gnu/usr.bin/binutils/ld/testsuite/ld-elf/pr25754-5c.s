	.text
	.p2align 4
	.globl	get_bar
	.type	get_bar, @function
get_bar:
	.cfi_startproc
	movq	bar@GOTPCREL(%rip), %rax
	ret
	.cfi_endproc
	.size	get_bar, .-get_bar
	bar = 0xfffffff0
	.section	.note.GNU-stack,"",@progbits
