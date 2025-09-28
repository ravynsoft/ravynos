	.syntax	unified
	.cpu	cortex-a8

	.text
	.align	12
	
_start:
	.type	_start,%function
	bx	lr
	.size	_start,.-_start

	.align	8
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
	blx	_func
	.size	_test,.-_test

# We have no mapping symbols for stubs.  This make the disassembler
# list the stub correctly in ARM mode.
	.align	2
	.arm

# Align stub table for address matching.
	.align	12

