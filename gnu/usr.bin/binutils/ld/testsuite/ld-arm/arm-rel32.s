	.globl	foo
foo:
	mov	pc,lr
	bal	foo(PLT)

	.data
	.word	foo-.
	.word	foo-.+0x100
