	.type foo, %gnu_indirect_function
	.global __GI_foo
	.hidden __GI_foo
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
	call	__GI_foo
	leaq	__GI_foo(%rip), %rax
	ret
	.size	bar, .-bar
