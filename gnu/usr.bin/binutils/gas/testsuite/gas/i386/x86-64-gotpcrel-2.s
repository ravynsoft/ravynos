	.text
foo:
	movq	foo@GOTPCREL(%rip), %rax
