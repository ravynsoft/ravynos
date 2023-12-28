	.text
	.globl bar
	.type	bar, @function
bar:
	jmp	foo
	.size	bar, .-bar
	.hidden	foo
	.type foo, %gnu_indirect_function
	.globl foo
foo:
	ret
	.size	foo, .-foo
