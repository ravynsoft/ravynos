	.text
	.align	2
	.global	_start
_start:
	ldr	x0, [x0, #:got_lo12:_start]
