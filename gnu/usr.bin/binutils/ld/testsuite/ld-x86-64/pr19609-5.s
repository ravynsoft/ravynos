	.text
	.weak bar
	.globl	_start
	.type	_start, @function
_start:
	call	*bar@GOTPCREL(%rip)
