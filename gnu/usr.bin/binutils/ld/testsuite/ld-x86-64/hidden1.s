	.text
.globl bar
	.type	bar, @function
bar:
	leaq	foo(%rip), %rax
	ret
	.size	bar, .-bar
	.weak	foo
	.hidden	foo
