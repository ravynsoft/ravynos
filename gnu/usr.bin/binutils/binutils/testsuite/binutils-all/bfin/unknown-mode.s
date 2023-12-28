	.text

	// Below 4 bytes are "A1 += R6.H * R4.L (IS);"
	// with a invalid mode "0xf".
	.byte 0xe1
	.byte 0xc1
	.byte 0x34
	.byte 0x98
