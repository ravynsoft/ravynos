	.text
.globl bar
	.type	bar, @function
bar:
	movabsq	$foo@GOTOFF, %rax
	.size	bar, .-bar
	.weak	foo
	.hidden	foo
