	.extInstruction insn1, 7, 0x21, SUFFIX_NONE, SYNTAX_3OP
	.extInstruction insn2, 7, 0x21, SUFFIX_NONE, SYNTAX_2OP
	.extInstruction insn3, 7, 0x21, SUFFIX_NONE, SYNTAX_1OP
	.extInstruction insn4, 7, 0x21, SUFFIX_NONE, SYNTAX_NOP

start:
	insn1	r0,r1,r2
	insn2	r0,r1
	insn3	r1
	insn4
