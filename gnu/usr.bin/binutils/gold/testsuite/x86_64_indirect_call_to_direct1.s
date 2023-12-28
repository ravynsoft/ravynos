	.text
	.globl	foo
	.type	foo, @function
foo:
	ret
	.size	foo, .-foo
	.globl	main
	.type	main, @function
main:
	call	*foo@GOTPCREL(%rip)
	ret
	.size	main, .-main
