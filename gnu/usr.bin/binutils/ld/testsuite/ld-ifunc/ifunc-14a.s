	.text
	.globl bar
	.type	bar, @function
bar:
	jmp	foo@PLT
	.size	bar, .-bar
	.hidden	foo
