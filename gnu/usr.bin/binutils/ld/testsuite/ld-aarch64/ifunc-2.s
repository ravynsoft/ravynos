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
	bl	__GI_foo
        adrp    x0, __GI_foo
        add     x0, x0, :lo12:__GI_foo
	ret
	.size	bar, .-bar
