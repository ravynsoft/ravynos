	.text
	.globl	foo
	.type	foo, @function
foo:
	ret
	.size	foo, .-foo
	.globl	main
	.type	main, @function
main:
	jmp	*foo@GOTPCREL(%rip)
	.size	main, .-main
