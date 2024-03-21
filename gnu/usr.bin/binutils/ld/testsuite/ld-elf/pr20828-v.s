	.text
	.globl	new_foo
	.type	new_foo, %function
new_foo:
	.size	new_foo, . - new_foo
	.symver	new_foo, foo@@ver_foo

	.data
	.globl	bar
	.type	bar, %object
bar:
	.dc.a	foo
	.size	bar, . - bar
