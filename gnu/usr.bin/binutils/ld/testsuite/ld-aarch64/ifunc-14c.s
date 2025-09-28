	.text
	.globl xxx
	.type	xxx, @function
xxx:
	bl	foo
	.size	xxx, .-xxx
	.hidden	foo
