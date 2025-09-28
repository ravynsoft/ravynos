# arm_bl_out_of_range.s
# Test ARM bl instructions just out of the branch range limits.
	.syntax	unified
	.arch	armv5te

	.section	.text.pre,"x"

# Add padding so that target is just out of branch range. 
	.space	8

	.align	2
	.global	_backward_target
_backward_target:
	bx	lr
	.size	_backward_target, .-_backward_target
	
	.text
# Use 256-byte alignment so that we know where the stubs start.
	.align	8

# Define _start so that linker does not complain.
	.global	_start
_start:
	bx	lr
	.size	_start, .-_start

	.global	_backward_test
_backward_test:
	bl	_backward_target
	.size	_backward_test, .-_backward_test

	.global	_forward_test
_forward_test:
	bl	_forward_target
	.size	_forward_test, .-_forward_test
	
# Align stub table for address matching
	.align	8

	.section	.text.post,"x"

# Add padding so that target is just out of branch range. 
	.space	16

	.align	2
	.global	_forward_target
_forward_target:
	bx	lr
	.size	_forward_target, .-_forward_target
