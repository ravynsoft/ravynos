# Test 1OP and NOP syntax

	.extInstruction	noop, 0x07, 0x10, SUFFIX_FLAG, SYNTAX_NOP
	.extInstruction	myinsn, 0x07, 0x3E, SUFFIX_FLAG, SYNTAX_1OP

	myinsn	r0
	myinsn	0x3A
	myinsn	0xdeadbeef
	myinsn	@label
	myinsn	@label@pcl

	noop
