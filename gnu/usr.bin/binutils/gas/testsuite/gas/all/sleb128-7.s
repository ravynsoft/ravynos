	.text
_L1:
	.byte 0
	.byte 0
	.byte 0
_L2:

	.data
	.sleb128 200+(_L2 - _L1)
	.byte 42
	.sleb128 200+(_L1 - _L2)
	.byte 42
	.sleb128 (_L2 - _L1)+200
	.byte 42
	.sleb128 (_L1 - _L2)+200
	.byte 42
