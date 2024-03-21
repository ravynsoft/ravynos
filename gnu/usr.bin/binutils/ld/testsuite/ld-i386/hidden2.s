	.text
.globl bar
	.type	bar, @function
bar:
	call	foo
	ret
	.size	bar, .-bar
	.weak	foo
	.hidden	foo
