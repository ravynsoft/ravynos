	.text
	.globl	foo
	.type	foo, @function
foo:
	.word	0
	movab	protected_foo, %r0
	movab	hidden_foo, %r0
	movab	internal_foo, %r0
	ret
	.size	foo, . - foo
