	.text
	.globl foo
	.type foo, @function
foo:
	.cfi_startproc
	ret
	.cfi_endproc
	.size	foo, .-foo
