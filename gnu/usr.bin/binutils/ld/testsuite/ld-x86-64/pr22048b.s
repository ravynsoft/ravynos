	.text
	.globl _start
	.type	_start, @function
_start:
	.cfi_startproc
	rep ret
	.cfi_endproc
	.size	_start, .-_start
