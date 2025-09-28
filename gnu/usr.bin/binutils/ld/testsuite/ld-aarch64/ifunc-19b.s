	.text
	.type foo, %gnu_indirect_function
	.hidden foo
	.globl foo
foo:
	ret
	.size	foo, .-foo
	.globl bar
bar:
	bl	foo1
	ret
	.size	bar, .-bar
	.hidden foo1
	.globl foo1
	foo1 = foo
