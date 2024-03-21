.globl __GI_foo
	.set __GI_foo, foo
	.text
.globl foo
	.type	foo, %function
foo:
	.byte	0x0
	.size	foo, .-foo
	.type foo, %gnu_indirect_function
