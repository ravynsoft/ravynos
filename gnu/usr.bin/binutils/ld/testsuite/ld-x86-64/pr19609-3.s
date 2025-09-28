	.data
foo:
	.quad	0
	.text
	.globl  _start
	.type	_start, @function
_start:
	cmpl	foo@GOTPCREL(%rip), %eax
	cmpl	foo@GOTPCREL(%rip), %r11d
	.size	_start, .-_start
