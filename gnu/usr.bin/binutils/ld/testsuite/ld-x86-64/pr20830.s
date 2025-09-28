	.text
	.globl foo
	.type foo, @function
foo:
	.cfi_startproc
	call	func@plt
	movq	func@GOTPCREL(%rip), %rax
	.cfi_endproc
