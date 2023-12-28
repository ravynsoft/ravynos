	.type foo, %gnu_indirect_function
	.set __GI_foo, foo
	.text
	.type	foo, @function
foo:
	ret
	.size	foo, .-foo
.globl bar
	.type	bar, @function
bar:
	call	__GI_foo@PLT
	ret
	.size	bar, .-bar
