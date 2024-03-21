	.text
	.protected	foo
	.globl foo
	.type	foo, @function
foo:
	ret
	.size	foo, .-foo
	.globl bar
	.type	bar, @function
bar:
	movl	foo@GOTOFF(%ecx), %eax
	.size	bar, .-bar
