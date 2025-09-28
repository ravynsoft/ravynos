	.text
	.globl bar
	.type	bar, @function
bar:
	bl	foo
	.size	bar, .-bar
	.hidden	foo
