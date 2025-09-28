	.text
	.type foo, %gnu_indirect_function
.globl foo
	.type	foo, @function
foo:
	ret
	.size	foo, .-foo
