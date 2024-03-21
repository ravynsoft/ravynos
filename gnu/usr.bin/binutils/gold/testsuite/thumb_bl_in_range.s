# thumb_bl_in_range.s
# Test THUMB/THUMB-2 bl instructions just within the branch range limits.
	.syntax	unified

	.section	.text.pre,"x"

# Add padding so that target is just in branch range. 
	.space	8

	.global	_backward_target
	.code	16
	.thumb_func
	.type	_backword_target, %function
_backward_target:
	bx	lr
	.size	_backward_target, .-_backward_target
	
	.text

# Define _start so that linker does not complain.
	.global	_start
	.code	32
	.align	2
	.type	_start, %function
_start:
	bx	lr
	.size	_start, .-_start

	.global	_backward_test
	.code	16
	.thumb_func
	.type	_backward_test, %function
_backward_test:
	bl	_backward_target
	.size	_backward_test, .-_backward_test

	.global	_forward_test
	.code	16
	.thumb_func
	.type	_forward_test, %function
_forward_test:
	bl	_forward_target
	.size	_forward_test, .-_forward_test
	
	.section	.text.post,"x"

# Add padding so that target is just in branch range. 
	.space	10

	.global	_forward_target
	.code	16
	.thumb_func
	.type	_forward_target, %function
_forward_target:
	bx	lr
	.size	_forward_target, .-_forward_target
