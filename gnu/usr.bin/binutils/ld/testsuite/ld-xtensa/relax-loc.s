	.file	1 "relax-loc.s"
	.globl	_start
	.globl	_ResetVector
	.text
_ResetVector:
_start:
	.loc	1 1
	j	1f
	.literal_position
1:
	.loc	1 2

	.rep	10000
	movi	a2, 0x12345678
	.endr
