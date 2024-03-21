	.text
	.align	4
	.literal	.L0, 0x12345
	.literal	.L1, 0x78f078f0

f:
	l32r	a2, .L0
	.rep	44000
	_nop
	_nop
	.endr
	l32r	a2, .L1
	ret
