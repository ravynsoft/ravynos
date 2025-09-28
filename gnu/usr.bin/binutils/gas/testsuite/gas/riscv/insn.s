target:
	.insn r  0x33,  0,  0, a0, a1, a2
	.insn i  0x13,  0, a0, a1, 13
	.insn i  0x67,  0, a0, 10(a1)
	.insn i   0x3,  0, a0, 4(a1)
	.insn sb 0x63,  0, a0, a1, target
	.insn b  0x63,  0, a0, a1, target
	.insn s  0x23,  0, a0, 4(a1)
	.insn u  0x37, a0, 0xfff
	.insn uj 0x6f, a0, target
	.insn j  0x6f, a0, target

	.insn cr  0x2, 0x8, a0, a1
	.insn ci  0x1, 0x0, a0, 4
	.insn ciw 0x0, 0x0, a1, 1
	.insn css 0x2, 0x6, a0, 1
	.insn cl  0x0, 0x2, a0, 1(a1)
	.insn cs  0x0, 0x6, a0, 1(a1)
	.insn cb  0x1, 0x6, a1, target
	.insn cj  0x1, 0x5, target

	.insn r  OP,  0,  0, a0, a1, a2
	.insn i  OP_IMM,  0, a0, a1, 13
	.insn i  JALR,  0, a0, 10(a1)
	.insn i  LOAD,  0, a0, 4(a1)
	.insn sb BRANCH,  0, a0, a1, target
	.insn b  BRANCH,  0, a0, a1, target
	.insn s  STORE,  0, a0, 4(a1)
	.insn u  LUI, a0, 0xfff
	.insn uj JAL, a0, target
	.insn j  JAL, a0, target

	.insn cr  C2, 0x8, a0, a1
	.insn ci  C1, 0x0, a0, 4
	.insn ciw C0, 0x0, a1, 1
	.insn css C2, 0x6, a0, 1
	.insn cl  C0, 0x2, a0, 1(a1)
	.insn cs  C0, 0x6, a0, 1(a1)
	.insn ca  C1, 0x23, 0x3, a0, a1
	.insn cb  C1, 0x6, a1, target
	.insn cj  C1, 0x5, target

	.insn r  MADD, 0, 0, a0, a1, a2, a3
	.insn r4 MADD, 0, 0, a0, a1, a2, a3
	.insn r4 MADD, 0, 0, fa0, a1, a2, a3
	.insn r4 MADD, 0, 0, fa0, fa1, a2, a3
	.insn r4 MADD, 0, 0, fa0, fa1, fa2, a3
	.insn r4 MADD, 0, 0, fa0, fa1, fa2, fa3
	.insn r  0x33,  0,  0, fa0, a1, a2
	.insn r  0x33,  0,  0, a0, fa1, a2
	.insn r  0x33,  0,  0, fa0, fa1, a2
	.insn r  0x33,  0,  0, a0, a1, fa2
	.insn r  0x33,  0,  0, fa0, a1, fa2
	.insn r  0x33,  0,  0, a0, fa1, fa2
	.insn r  0x33,  0,  0, fa0, fa1, fa2

	.insn r  OP_V, 0, 1, x1, x3, x2

	.insn 0x0001
	.insn 0x00000013
	.insn 0x0000001f
	.insn 0x0000003f
	.insn 0x007f
	.insn 0x107f
	.insn 0x607f
	.insn 0x2, 0x0001
	.insn 0x4, 0x00000013
	.insn 6, 0x0000001f
	.insn 8, 0x0000003f
	.insn 10, 0x007f
	.insn 12, 0x107f
	.insn 22, 0x607f

	.insn 0x8000000000000000007f
	.insn 10, 0x8000000000000000007f
	.insn 0x000000000000fedcba98765432100123456789ab607f
	.insn 22, 0x000000000000fedcba98765432100123456789ab607f
	.insn 0x00dcba98765432100123456789abcdef55aa33cc607f
	.insn 22, 0x00dcba98765432100123456789abcdef55aa33cc607f
	.insn 0xfedcba98765432100123456789abcdef55aa33cc607f
	.insn 22, 0xfedcba98765432100123456789abcdef55aa33cc607f
