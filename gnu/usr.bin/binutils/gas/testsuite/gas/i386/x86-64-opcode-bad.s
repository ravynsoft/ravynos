	.text
# All the followings are bad opcodes for x86-64.
	.insn VEX.L1.0f 0x46, %k5, %r10d, %k6
	.insn VEX.L1.0f 0x46, %k5, %r10d, %r14d
