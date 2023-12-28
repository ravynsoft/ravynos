	.text
	.weak	foobar
	.globl  _start
	.type	_start, @function
_start:
	call	*foobar@GOTPCREL(%rip)
	.size	_start, .-_start
