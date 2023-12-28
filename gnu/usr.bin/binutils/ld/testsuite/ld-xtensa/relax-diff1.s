	.globl	_start
	.globl	_ResetVector
	.text
_ResetVector:
_start:
	.literal_position
	movi	a2, 0x12345678
	movi	a2, 0x12345678
1:
	.space	250
2:
	.space	65530
3:
	.align	4
	.byte	1b - 2b
	.byte	2b - 1b
	.short	2b - 3b
	.short	3b - 2b
