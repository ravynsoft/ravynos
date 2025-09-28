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
	movabsq	$foo@GOTOFF, %rax
	.size	bar, .-bar
