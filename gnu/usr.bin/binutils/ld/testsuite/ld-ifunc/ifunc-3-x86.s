	.type foo, %gnu_indirect_function
	.global __GI_foo
	.protected __GI_foo
	.set __GI_foo, foo
	.text
.globl foo
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
