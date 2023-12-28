	.text
foo:
	.quad 0
	movq	foo@GOTPCREL(%rip), %rax
	movq	bar@GOTPCREL(%rip), %rax
	bar = 0xfffffff0
