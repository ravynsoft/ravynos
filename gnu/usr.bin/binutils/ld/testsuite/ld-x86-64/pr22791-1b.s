	.text
	.globl	main
	.type	main, @function
main:
	movl	foo(%rip), %eax
	.size	main, .-main
	.section	.note.GNU-stack
