	.text
	.globl  _start
	.type	_start, @function
_start:
	movq	foobar@GOTPCREL(%rip), %rax
	.size	_start, .-_start
