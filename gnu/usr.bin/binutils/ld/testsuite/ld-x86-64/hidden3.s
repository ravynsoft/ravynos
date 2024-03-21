	.text
.globl bar
	.type	bar, @function
bar:
	leaq	foo(%rip), %rax
	ret
	.size	bar, .-bar
	.hidden	foo
