	.data
foo:
	.quad	0
	.text
	.globl  _start
	.type	_start, @function
_start:
	movq	foo@GOTPCREL(%rip), %rax
	movq	foo@GOTPCREL(%rip), %r11
	.size	_start, .-_start
