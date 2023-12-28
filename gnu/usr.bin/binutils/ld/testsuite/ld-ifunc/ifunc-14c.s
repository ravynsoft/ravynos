	.text
	.globl xxx
	.type	xxx, @function
xxx:
	jmp	foo@PLT
	.size	xxx, .-xxx
	.hidden	foo
