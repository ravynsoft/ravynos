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
	movabsq	$foo@GOTOFF, %rax
	.size	bar, .-bar
