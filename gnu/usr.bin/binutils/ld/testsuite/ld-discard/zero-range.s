	.text
	.globl _start
_start:

	.section	.debug_ranges
	.long	.Ltext
	.long	.Ltext + 2
	.long	0
	.long	0

	.section	.text.exit,"ax"
.Ltext:
	.long	0
