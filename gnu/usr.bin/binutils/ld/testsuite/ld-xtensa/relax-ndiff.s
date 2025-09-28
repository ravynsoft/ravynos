	.globl	_start
	.globl	_ResetVector
	.text
_ResetVector:
_start:
	.literal_position
	movi	a2, 0x12345678
	movi	a2, 0x12345678
1:
	.space	10
2:
	.space	10
3:
	.align	4
	.word	1b - 2b
	.word	3b - 2b
	.short	1b - 2b
	.short	3b - 2b
	.byte	1b - 2b
	.byte	3b - 2b
