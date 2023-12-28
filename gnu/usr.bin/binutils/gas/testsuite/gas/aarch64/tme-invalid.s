// Instructions in this file are invalid.
// Other files provide more extensive testing of valid instructions;

# tcancel only accept 16bit unsigned constant immediate.
1:
	tcancel	-1
	tcancel	65536
	tcancel	0x10000
	tcancel	1b

# tcancel doesn't accept any register.
	tcancel w1
	tcancel x1
	tcancel w23
	tcancel x23
	tcancel wzr
	tcancel xzr
	tcancel wsp
	tcancel xsp
	tcancel sp

# tstart must has one X register operand.
	tstart
	tstart w1
	tstart w17
	tstart wzr
	tstart wsp
	tstart xsp
