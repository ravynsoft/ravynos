# thumb_bl_out_of_range_local.s
# Test THUMB/THUMB-2 bl instructions just out of the branch range limits
# and with local branch targets.
	.syntax	unified

	.section	.text.pre,"x"

# Add padding so that target is just output of branch range. 
	.space	6

	.code	16
	.thumb_func
	.type	.Lbackward_target, %function
.Lbackward_target:
	bx	lr
	.size	.Lbackward_target, .-.Lbackward_target
	
	.text
# Use 256-byte alignment so that we know where the stubs start.
	.align	8

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
	bl	.Lbackward_target
	.size	_backward_test, .-_backward_test

	.global	_forward_test
	.code	16
	.thumb_func
	.type	_forward_test, %function
_forward_test:
	bl	.Lforward_target
	.size	_forward_test, .-_forward_test

# Switch back to ARM mode so that we can see stubs
        .align  2
	.code	32

# Align stub table for address matching.
        .align  8
	
	.section	.text.post,"x"

# Add padding so that target is just out of branch range. 
	.space	12

	.code	16
	.thumb_func
	.type	.Lforward_target, %function
.Lforward_target:
	bx	lr
	.size	.Lforward_target, .-.Lforward_target
