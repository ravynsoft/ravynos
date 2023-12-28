	.text
	.symver foo,foo@FOO
	.globl	foo
	.type	foo, @function
foo:
	movq foobar@GOTPCREL(%rip), %rax
	.size	foo, .-foo
	.globl	bar
	.type	bar, @function
bar:
	jmp	*foo@GOTPCREL(%rip)
	.size	bar, .-bar
	.comm foobar,30,4
