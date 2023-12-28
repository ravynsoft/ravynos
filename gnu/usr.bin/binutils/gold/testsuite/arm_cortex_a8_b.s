	.syntax	unified
	.cpu	cortex-a8

	.text
	.align	12
	
_start:
	.type	_start,%function
	bx	lr
	.size	_start,.-_start

	.align	8
	.thumb
	.global	_func
	.type	_func,%function
_func:
	bx	lr
	.size	_func,.-_func

	.align	11
	.space	2042

	.align	1
	.thumb
	.global	_test
	.type	_test,%function
_test:
	add.w	r0, r0, 0
	b.w	_func
	.size	_test,.-_test

# Align stub table for address matching.
	.align	12
