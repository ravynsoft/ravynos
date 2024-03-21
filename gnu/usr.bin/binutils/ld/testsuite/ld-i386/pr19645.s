	.text
	.global bar
	.type bar,"function"
bar:
	movl foobar@GOT(%ebx), %eax
	.comm foobar,30,4
