	.text

	.type	foo_resolver, @function
foo_resolver:
	ret
	.size	foo_resolver, .-foo_resolver

	# The ifunc `foo` is called by the ifunc-caller.
	.globl	foo
	.type	foo, %gnu_indirect_function
	.set	foo, foo_resolver
