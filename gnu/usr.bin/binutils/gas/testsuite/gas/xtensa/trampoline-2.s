	.text

	.rep 4000
	bnez	a2, .L1
	.endr

	.rep 100000
	_nop
	.endr

.L1:
	j	.L1

	.rep 100000
	_nop
	.endr

	.rep 4000
	bnez	a2, .L1
	.endr
