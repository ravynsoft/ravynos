	.text
	.globl	orig
	.type	orig, @function
orig:
	xorl	%eax, %eax
	ret
	.size	orig, .-orig
	.section	.text.startup,"ax",@progbits
	.globl	main
	.type	main, @function
main:
	xorl	%eax, %eax
	ret
	.size	main, .-main
