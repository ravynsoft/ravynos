	.type	bar, @function
bar:
	movl	_DYNAMIC@GOT(%ecx), %eax
	.size	bar, .-bar
