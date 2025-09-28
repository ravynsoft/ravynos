	.text
.globl bar
	.type	bar, @function
bar:
	leal	foo@GOTOFF(%ecx), %eax
	ret
	.size	bar, .-bar
	.hidden	foo
