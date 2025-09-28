	.text
_L1:
	.byte 0
	.byte 0
	.byte 0
_L2:

	.data
	.sleb128 _L1 - _L2
	.byte 42
