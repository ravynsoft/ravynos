	.section	.tbss,"awT",@nobits
	.align 4
	.type	a, @object
	.size	a, 4
a:
	.zero	4
	.text
.globl _start
	.type	_start, @function
_start:
	leal	a@dtpoff, %eax
	.size	_start, .-_start
