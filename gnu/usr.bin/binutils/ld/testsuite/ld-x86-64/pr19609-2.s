	.data
foo:
	.quad	0
	.text
	.globl  _start
	.type	_start, @function
_start:
	cmpq	foo@GOTPCREL(%rip), %rax
	.size	_start, .-_start
