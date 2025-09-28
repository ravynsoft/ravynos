# Check Illegal 64bit AMX-COMPLEX instructions

.text
	#tcmmimfp16ps %tmm4,%tmm5,%tmm6 set VEX.W = 1 (illegal value).
	.insn VEX.128.66.0F38.W1 0x6c, %tmm5, %tmm4, %tmm6

	#tcmmimfp16ps %tmm4,%tmm4,%tmm6 set VEX.L = 1 (illegal value).
	.insn VEX.256.66.0F38.W0 0x6c, %tmm5, %tmm4, %tmm6

	#tcmmimfp16ps %tmm4,%tmm5,%tmm6 set VEX.R = 0 (illegal value).
	.insn VEX.128.66.0F38.W0 0x6c, %xmm5, %xmm4, %xmm14

	#tcmmimfp16ps %tmm4,%tmm5,%tmm6 set VEX.B = 0 (illegal value).
	.insn VEX.128.66.0F38.W0 0x6c, %xmm13, %xmm4, %xmm6

	#tcmmimfp16ps %tmm4,%tmm5,%tmm6 set VEX.VVVV = 0110 (illegal value).
	.insn VEX.128.66.0F38.W0 0x6c, %xmm5, %xmm9, %xmm6
