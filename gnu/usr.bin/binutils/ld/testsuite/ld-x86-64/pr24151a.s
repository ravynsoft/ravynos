	.text
	.globl	bar
	.type	bar,@function
bar:
	movl	$30, foo(%rip)
	.size	bar, .-bar
	.protected foo
	.type	foo,@object
	.comm	foo,4,4
