	.text
.globl foo
	.protected	foo
	.type	foo, @function
foo:
	ret
	.size	foo, .-foo
.globl bar
	.type	bar, @function
bar:
	leal	foo@GOTOFF(%ecx), %eax
	ret
	.size	bar, .-bar
