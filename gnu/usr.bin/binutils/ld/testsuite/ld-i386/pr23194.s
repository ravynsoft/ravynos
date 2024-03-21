	.text
	.symver foo,foo@FOO
	.globl	foo
	.type	foo, @function
foo:
	movl foobar@GOT(%ebx), %eax
	.size	foo, .-foo
	.globl	bar
	.type	bar, @function
bar:
	jmp	*foo@GOT(%eax)
	.size	bar, .-bar
	.comm foobar,30,4
