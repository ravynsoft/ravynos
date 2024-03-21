# Check Illegal 64bit AMX-FP16 instructions

.text
	#tdpfp16ps %tmm5,%tmm4,%tmm3 set VEX.W = 1 (illegal value).
	.insn VEX.128.F2.0F38.W1 0x5c, %tmm4, %tmm5, %tmm3
	.fill 0x05, 0x01, 0x90

	#tdpfp16ps %tmm5,%tmm4,%tmm3 set VEX.L = 1 (illegal value).
	.insn VEX.256.F2.0F38.W0 0x5c, %tmm4, %tmm5, %tmm3
	.fill 0x05, 0x01, 0x90

	#tdpfp16ps %tmm5,%tmm4,%tmm3 set VEX.R = 0 (illegal value).
	.insn VEX.128.F2.0F38.W0 0x5c, %xmm4, %xmm5, %xmm11

	#tdpfp16ps %tmm5,%tmm4,%tmm3 set VEX.B = 0 (illegal value).
	.insn VEX.128.F2.0F38.W0 0x5c, %xmm12, %xmm5, %xmm3

	#tdpfp16ps %tmm5,%tmm4,%tmm3 set VEX.VVVV = 0110 (illegal value).
	.insn VEX.128.F2.0F38.W0 0x5c, %xmm4, %xmm9, %xmm3
