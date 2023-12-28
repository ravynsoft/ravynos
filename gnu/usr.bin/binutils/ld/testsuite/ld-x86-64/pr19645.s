	.text
	.global bar
	.type bar,"function"
bar:
	movq foobar@GOTPCREL(%rip), %rax
	.comm foobar,30,4
