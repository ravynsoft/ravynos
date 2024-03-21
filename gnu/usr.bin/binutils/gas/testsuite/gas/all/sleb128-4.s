	.text
_L1:
	.byte 0
	.byte 0
	.byte 0
_L2:

	.data
	.sleb128 _L2 - _L1 + (1 << 31)
	.byte 42
