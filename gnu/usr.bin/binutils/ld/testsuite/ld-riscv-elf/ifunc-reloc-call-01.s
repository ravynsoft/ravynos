	.text

	.type	foo_resolver, @function
foo_resolver:
	ret
	.size	foo_resolver, .-foo_resolver

	.globl	foo
	.type	foo, %gnu_indirect_function
	.set	foo, foo_resolver

	.globl	bar
	.type	bar, @function
bar:
	call	foo
	ret
	.size	bar, .-bar
