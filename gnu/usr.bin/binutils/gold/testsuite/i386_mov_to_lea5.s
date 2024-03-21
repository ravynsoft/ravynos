	.text
	.globl	foo
	.protected foo
	.type	foo, @function
foo:
	ret
	.size	foo, .-foo
	.globl	bar
	.type	bar, @function
bar:
	movl	foo@GOT(%ecx), %eax
	.size	bar, .-bar
