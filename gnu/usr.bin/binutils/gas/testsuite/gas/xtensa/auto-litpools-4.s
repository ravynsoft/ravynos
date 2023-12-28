	.section .init
	.literal a, 0x12345678
	movi	a2, 0x12345679
	.section .fini
	.literal b, 0x1234567a
	movi	a2, 0x1234567b
