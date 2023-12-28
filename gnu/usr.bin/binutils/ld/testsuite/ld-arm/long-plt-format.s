	.globl	_start
	.type	_start,%function
	.globl foo
_start:
	bl foo(PLT)
	.size	_start,.-_start

