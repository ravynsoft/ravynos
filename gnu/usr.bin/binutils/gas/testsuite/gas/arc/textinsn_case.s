	.cpu em
	.extInstruction aes_qRoundF, 7, 0x25, SUFFIX_COND, SYNTAX_3OP

	.text
test:
	aes_qRoundF  r1,r2,r3
