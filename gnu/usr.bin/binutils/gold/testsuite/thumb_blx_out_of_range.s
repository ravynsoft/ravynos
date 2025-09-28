# thumb_blx_out_of_range.s
# Test THUMB/THUMB-2 blx instructions just out of the branch range limits.
	.syntax	unified

	.section	.text.pre,"x"

# Add padding so that target is just output of branch range. 
	.space	4

	.global	_forward_target
	.global	_backward_target
	.type	_backword_target, %function
_backward_target:
	bx	lr
	.size	_backward_target, .-_backward_target
	
	.text
# Use 256-byte alignment so that we know where the stubs start.
	.align	8

# Define _start so that linker does not complain.
	.align	2
	.global	_start
	.code	32
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

	.align	2
	.global	_forward_test
	.code	16
	.thumb_func
	.type	_forward_test, %function
_forward_test:
	# Bit 1 of the BLX target comes from bit 1 of branch base address,
	# which is BLX instruction's address + 4.  We intentionally put this
	# forward BLX at an address n*4 + 2 so that the branch offset is
	# bumped up by 2.
	nop.n
	bl	_forward_target
	.size	_forward_test, .-_forward_test

# switch back to ARM mode so that stubs are disassembled correctly.
	.align	2
	.code	32
	
# Align stub table for address matching.
        .align  8

	.section	.text.post,"x"

# Add padding so that target is just out of branch range. 
	.space	12
	.align 2
	.code	32
	.global	_forward_target
	.type	_forward_target, %function
_forward_target:
	bx	lr
	.size	_forward_target, .-_forward_target
