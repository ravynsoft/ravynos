	.text
	.p2align 4,,15
	.globl	foo
	.type	foo, @function
foo:
	jmp	bar
	.size	foo, .-foo
	.section	.note.GNU-stack,"",@progbits
