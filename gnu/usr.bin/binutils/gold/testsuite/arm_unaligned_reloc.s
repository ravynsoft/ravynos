	.syntax	unified

	.global	_start
	.type	_start, %function
	.text
_start:
	bx	lr
	.size	_start,.-_start

	.section	.data.0,"aw",%progbits
	.align	12
	.type	x, %object
	.size	x, 4
x:
	.word	1

	.section	.data.1,"aw",%progbits
	.align	12

# This causes following relocations to be unaligned.
	.global	padding
	.type	padding, %object
	.size	padding, 1
padding:
	.byte	0

	.global	abs32
	.type	abs32, %object
	.size	abs32, 4
abs32:
	# We use x + 1 instead so that addend is non-zero
	# The disassembler sometimes skips repeating
	# zeros and prints "..." instead.
	.word	x + 1

	.global	rel32
	.type	rel32, %object
	.size	rel32, 4
rel32:
	.word	x + 1 - .

	.global	abs16
	.type	abs16, %object
	.size	abs16, 2
abs16:
	.short	x + 1
	.short	0
