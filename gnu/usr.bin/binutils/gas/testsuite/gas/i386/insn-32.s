	.text
insn:
	# nop
	.insn 0x90

	# pause
	.insn 0xf390
	.insn repe 0x90

	# fldz
	.insn 0xd9ee
	.insn 0xd9, $0xee

	# setssbsy
	.insn 0xf30f01e8

	# mov
	.insn 0x8b, %ecx, %eax
	.insn 0x8b, %ax, %cx
	.insn 0x89, %ecx, 4(%eax)
	.insn 0x8b, 0x4444(,%eax), %ecx

	# movzx
	.insn 0x0fb6, %ah, %cx
	.insn 0x0fb7, %eax, %ecx

	# xorb
	.insn lock 0x80/6, $1, %fs:(%eax)

	# bswap
	.insn 0x0fc8+r, %edx

1:
	# xbegin 3f
	.insn 0xc7f8, $3f-2f{:s32}
2:
	# loop 1b
	.insn 0xe2, $1b-3f{:s8}
3:

	# vzeroall
	.insn VEX.256.0F.WIG 0x77
	.insn {vex3} VEX.L1 0x0f77

	# vaddpd
	.insn VEX.66.0F 0x58, %xmm0, %xmm1, %xmm2
	.insn VEX.66 0x0f58, %ymm0, %ymm1, %ymm2

	# vaddss
	.insn VEX.LIG.F3.0F 0x58, %xmm0, %xmm1, %xmm2
	.insn EVEX.LIG.F3.0F.W0 0x58, 4(%eax){:d4}, %xmm1, %xmm2

	# vfmaddps
	.insn VEX.66.0F3A.W0 0x68, %xmm0, (%ecx), %xmm2, %xmm3
	.insn VEX.66.0F3A.W1 0x68, %xmm0, (%ecx), %xmm2, %xmm3
	.insn VEX.66.0F3A.W1 0x68, (%eax), %xmm1, %xmm2, %xmm3

	# vpermil2ps
	.insn VEX.66.0F3A.W0 0x48, $0, %xmm0, (%ecx), %xmm2, %xmm3
	.insn VEX.66.0F3A.W1 0x48, $2, %xmm0, (%ecx), %xmm2, %xmm3
	.insn VEX.66.0F3A.W1 0x48, $3, (%eax), %xmm1, %xmm2, %xmm3

	# kmovw
	.insn VEX.L0.0F.W0 0x92, %eax, %k1
	.insn VEX.L0.0F.W0 0x93, %k1, %eax

	# vaddps
	.insn EVEX.NP.0F.W0 0x58, {rn-sae}, %zmm0, %zmm1, %zmm2

	# vgather...
	.insn VEX.66.0f38.W0 0x92, %xmm0, (%eax, %xmm1, 2), %xmm3
	.insn EVEX.66.0f38.W1 0x93, (%eax, %xmm1, 2), %xmm3{%k4}

	# vexpandps
	.insn EVEX.66.0F38.W0 0x88, 4(%eax){:d4}, %ymm1

	# vcvtpd2phz
	.insn EVEX.512.66.M5.W1 0x5a, 64(%eax), %xmm0
	.insn EVEX.66.M5.W1 0x5a, 64(%eax), %zmm0
	.insn EVEX.66.M5.W1 0x5a, 64(%eax){:d64}, %xmm0
	.insn EVEX.512.66.M5.W1 0x5a, 8(%eax){1to8}, %xmm0
	.insn EVEX.66.M5.W1 0x5a, 8(%eax){1to8}, %zmm0
	.insn EVEX.66.M5.W1 0x5a, 8(%eax){1to8:d8}, %xmm0

	# vcvtph2pd
	.insn EVEX.M5.W0 0x5a, 16(%eax){:d16}, %zmm0
	.insn EVEX.M5.W0 0x5a, 2(%eax){1to8:d2}, %zmm0

	.intel_syntax noprefix
	# vfpclassps
	.insn EVEX.256.66.0f3a.W0 0x66, k0, [eax+32], 0xff
	.insn EVEX.66.0f3a.W0 0x66, k0, ymmword ptr [eax+32], 0xff
	.insn EVEX.256.66.0f3a.W0 0x66, k0, [eax+4]{1to8}, 0xff
	.insn EVEX.66.0f3a.W0 0x66, k0, dword ptr [eax+4]{1to8}, 0xff
