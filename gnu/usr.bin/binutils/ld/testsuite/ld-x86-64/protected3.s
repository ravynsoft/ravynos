	.protected	foo
.globl foo
	.data
	.align 4
	.type	foo, @object
	.size	foo, 4
foo:
	.long	1
	.text
.globl bar
	.type	bar, @function
bar:
	movq	foo@GOTPCREL(%rip), %rax
	movl	(%rax), %eax
	ret
	.size	bar, .-bar
