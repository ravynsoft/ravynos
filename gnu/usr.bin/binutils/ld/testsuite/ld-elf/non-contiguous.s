	.section .data.1, "a", %progbits
	# Fit in RAML
	.4byte 1
	.4byte 2
	.4byte 3

	.section .data.2, "a", %progbits
	# Fit in RAMU
	.4byte 4
	.4byte 5
	.4byte 6

	.section .data.3, "a", %progbits
	# Fit in RAMU
	.4byte 7
	.4byte 8

	.section .data.4, "a", %progbits
	# Fit in RAMZ
	.fill 0x3c, 1, 9

