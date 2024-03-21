.text
	#tdpbf16ps %tmm5,%tmm4,%tmm3 set VEX.W = 1 (illegal value).
	.insn VEX.128.F3.0F38.W1 0x5c, %tmm4, %tmm5, %tmm3
	.fill 0x05, 0x01, 0x90

	#tdpbf16ps %tmm5,%tmm4,%tmm3 set VEX.L = 1 (illegal value).
	.insn VEX.256.F3.0F38.W0 0x5c, %tmm4, %tmm5, %tmm3
	.fill 0x05, 0x01, 0x90

	#tdpbf16ps %tmm5,%tmm4,%tmm3 set VEX.R = 0 (illegal value).
	.insn VEX.128.F3.0F38.W0 0x5c, %xmm4, %xmm5, %xmm11

	#tdpbf16ps %tmm5,%tmm4,%tmm3 set VEX.B = 0 (illegal value).
	.insn VEX.128.F3.0F38.W0 0x5c, %xmm12, %xmm5, %xmm3

	#tdpbf16ps %tmm5,%tmm4,%tmm3 set VEX.VVVV = 0110 (illegal value).
	.insn VEX.128.F3.0F38.W0 0x5c, %xmm4, %xmm9, %xmm3

	#tileloadd (%rcx),%tmm1 set R/M= 001 (illegal value) without SIB.
	.insn VEX.128.F2.0F38.W0 0x4b, (%rcx), %xmm1

	#tdpbuud %tmm1,%tmm1,%tmm1 All 3 TMM registers can't be identical.
	.insn VEX.128.NP.0F38.W0 0x5e, %tmm1, %tmm1, %tmm1

	#tdpbuud %tmm0,%tmm1,%tmm1 All 3 TMM registers can't be identical.
	.insn VEX.128.NP.0F38.W0 0x5e, %tmm1, %tmm0, %tmm1

	#tdpbuud %tmm1,%tmm0,%tmm1 All 3 TMM registers can't be identical.
	.insn VEX.128.NP.0F38.W0 0x5e, %tmm0, %tmm1, %tmm1

	#tdpbuud %tmm1,%tmm1,%tmm0 All 3 TMM registers can't be identical.
	.insn VEX.128.NP.0F38.W0 0x5e, %tmm1, %tmm1, %tmm0
