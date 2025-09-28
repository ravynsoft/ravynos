	.syntax	unified
	.text

# Align this to 256-byte boundary for easier address matching.
	.align	8

# We do not want to run this file. We define _start here to avoid missing
# entry point.

	.global	_start
	.type	_start, %function
_start:
	bx	r0
	bx	r15
	.size	_start, .-_start

# Align this to 256-byte boundary for easier address matching.
	.align	8
