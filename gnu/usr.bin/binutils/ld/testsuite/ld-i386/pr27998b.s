	.text
	.p2align 4
	.type	my_foo, @function
my_foo:
	ret
	.size	my_foo, .-my_foo
	.p2align 4
	.type	resolve_foo, @function
resolve_foo:
	leal	my_foo@GOTOFF(%eax), %eax
	ret
	.size	resolve_foo, .-resolve_foo
	.type	foo, @gnu_indirect_function
	.set	foo,resolve_foo
	.p2align 4
	.globl	bar
	.type	bar, @function
bar:
	leal	foo@GOTOFF(%eax), %eax
	ret
