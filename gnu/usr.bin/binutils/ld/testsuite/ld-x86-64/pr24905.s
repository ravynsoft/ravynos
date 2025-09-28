	.text
	.globl	foo
	.type	foo, @function
foo:
	leaq	EXTERNAL_SYM(%rip), %rdi
	.hidden	EXTERNAL_SYM
