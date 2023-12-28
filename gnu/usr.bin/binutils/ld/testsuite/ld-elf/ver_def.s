	.data
	.globl	new_foo
	.type	new_foo, %object
new_foo:
	.symver	new_foo, foo@@ver_foo
