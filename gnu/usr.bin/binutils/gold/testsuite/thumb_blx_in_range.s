# thumb_blx_in_range.s
#
# Test THUMB/THUMB-2 blx instructions just within the branch range limits.
# Because bit 1 of the branch target comes from the branch instruction
# address, the branch range from PC (branch instruction address + 4) is
# acutally -((1<<22) + 2) to ((1<<22) - 4) for THUMB and -((1<<24) + 2) to
# ((1<<24) - 4) from THUMB2.

	.syntax	unified
	.section	.text.pre,"x"

# Add padding so that target is just in branch range. 
	.space	8

	.align	2
	.global	_backward_target
	.code	32
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
	nop.n
	blx	_backward_target
	.size	_backward_test, .-_backward_test

	.align	2
	.global	_forward_test
	.code	16
	.thumb_func
	.type	_forward_test, %function
_forward_test:
	blx	_forward_target
	.size	_forward_test, .-_forward_test
	.code	32
	
	.section	.text.post,"x"

# Add padding so that target is just in branch range. 
	.space	12

	.align	2
	.global	_forward_target
	.code	32
	.type	_forward_target, %function
_forward_target:
	bx	lr
	.size	_forward_target, .-_forward_target
