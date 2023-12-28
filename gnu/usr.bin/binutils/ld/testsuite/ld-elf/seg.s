	.section boot,"ax"
	.4byte 0x76543210
	.section reset,"ax"
	.4byte 0xfedcba98
	.text
	.4byte 0x12345678
