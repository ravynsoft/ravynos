	.text
.globl _start
	.type	_start, @function
_start:
	cmpq	$0, foo(%rip)
	.size	_start, .-_start
