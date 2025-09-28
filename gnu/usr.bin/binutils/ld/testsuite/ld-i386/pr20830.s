	.text
	.globl foo
	.type foo, @function
foo:
	.cfi_startproc
	call	func@plt
	movl	func@GOT(%ebx), %eax
	.cfi_endproc
