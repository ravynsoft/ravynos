	.data
	.protected	foo
	.globl foo
	.align 4
	.type	foo, @object
	.size	foo, 4
foo:
	.long	1
	.text
	.globl bar
	.type	bar, @function
bar:
	movl	foo@GOTOFF(%ecx), %eax
	.size	bar, .-bar
