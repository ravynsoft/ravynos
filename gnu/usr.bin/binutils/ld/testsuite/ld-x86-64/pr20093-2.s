	.largecomm	foo1,1073741824,32
	.largecomm	foo2,1073741824,32
	.largecomm	foo3,1073741824,32
	.text
	.globl	_start
	.type	_start, @function
_start:
	movq	foo1@GOTPCREL(%rip), %rax
	.size	_start, .-_start
