	.type	bar, @function
bar:
	ret
	.size	bar, .-bar
.globl foo
	.type	foo, @function
foo:
	leal	bar, %eax
	ret
	.size	foo, .-foo
