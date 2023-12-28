	.type	bar, @function
bar:
	ret
	.size	bar, .-bar
.globl foo
	.type	foo, @function
foo:
	leaq	bar(%rip), %rax
	ret
	.size	foo, .-foo
