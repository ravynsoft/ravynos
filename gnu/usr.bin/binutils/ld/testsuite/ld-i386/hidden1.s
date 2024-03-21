	.text
.globl bar
	.type	bar, @function
bar:
	leal	foo@GOTOFF(%ecx), %eax
	ret
	.size	bar, .-bar
	.weak	foo
	.hidden	foo
