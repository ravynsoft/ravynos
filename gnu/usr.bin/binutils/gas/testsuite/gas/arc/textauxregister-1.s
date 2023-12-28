	.extAuxRegister myreg1, 0x80018000, r|w
	.extAuxRegister myreg2, -256, r|w

	lr	r2, [myreg1]
	lr	r1, [myreg2]
