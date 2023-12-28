	.text
	.globl foo
	.type foo, @function
foo:
	.cfi_startproc
	jmp	 bar@PLT
	.cfi_endproc
	.size	foo, .-foo
