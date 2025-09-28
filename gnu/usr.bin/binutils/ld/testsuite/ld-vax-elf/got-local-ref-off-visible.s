	.text
	.globl	foo
	.type	foo, @function
foo:
	.word	0
	movab	bar_hidden, %r0
	movab	bar_visible, %r1
	movab	bar_visible + 1, %r1
	movab	bar_visible + 2, %r1
	movab	baz, %r2
	ret
	.size	foo, . - foo
