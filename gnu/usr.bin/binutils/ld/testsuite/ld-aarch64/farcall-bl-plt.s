	.global _start
	.global foo
	.type foo, @function
	.text

_start:
	# ((1 << 25) - 1) << 2
	# call26 relocation out of range to plt stub,
	# we need long branch veneer.
	.skip 134217724, 0
	bl foo
	ret
