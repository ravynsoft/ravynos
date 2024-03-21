// fpmov.s Test file for AArch64 floating-point move instructions.

	.text

	// fp mov immediate
	fmov	s0, 12.0
	fmov	s0, 1.2e1
	fmov	s0, 0x41400000
	fmov	s0, -12.0
	fmov	s0, -1.2e1
	fmov	s0, 0xc1400000
	fmov	d0, -12.0
	fmov	d0, -1.2e1
	fmov	d0, 0xC028000000000000
	fmov	d0, 0.2421875
	fmov	d0, 0x3fcf000000000000
	fmov	s0, 0x3e780000

	fmov	d0, #2
	fmov	d0, #-2
	fmov	s0, 2
	fmov	s0, -2
