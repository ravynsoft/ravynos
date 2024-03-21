# Test for PRU 32-bit, 16 and 8-bit relocations

	.global byte_sym
	.global short_sym
	.global long_sym

	.set byte_sym, 0xFA
	.set short_sym, 0xFACE
	.set long_sym, 0xDEADBEEF
