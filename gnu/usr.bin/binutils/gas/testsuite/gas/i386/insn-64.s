	.text
insn:
	# nop
	.insn 0x90

	# pause
	.insn 0xf390
	.insn repe 0x90

	# fldz
	.insn 0xd9ee

	# setssbsy
	.insn 0xf30f01e8

	# mov
	.insn 0x8b, %ecx, %r8d
	.insn 0x8b, %rax, %rcx
	.insn 0x89, %ecx, 8(%r8)
	.insn 0x8b, 0x8080(,%r8), %ecx

	# movsx
	.insn 0x0fbe, %ah, %cx
	.insn 0x0fbf, %eax, %ecx
	.insn 0x63, %rax, %rcx

	# xorb
	.insn lock 0x80/6, $1, lock(%rip)

	# bswap
	.insn 0x0fc8+r, %rdx
	.insn 0x0fc8+r, %r8d

1:
	# xbegin 3f
	.insn 0xc7f8, $3f-2f{:s32}
2:
	# loop 1b
	.insn 0xe2, $1b-3f{:s8}
3:

	# add $var, %eax
	.insn 0x05, $var{:u32}

	# add $var, %rax
	.insn rex.w 0x05, $var{:s32}

	# cmpl (32-bit immediate split into two 16-bit halves)
	.insn 0x81/7, $0x1213, $0x2123, var(%rip)

	# vzeroall
	.insn VEX.256.0F.WIG 0x77
	.insn {vex3} VEX.L1 0x0f77

	# vaddpd
	.insn VEX.66.0F 0x58, %xmm8, %xmm1, %xmm2
	.insn VEX.66 0x0f58, %ymm0, %ymm9, %ymm2

	# vaddss
	.insn VEX.LIG.F3.0F 0x58, %xmm0, %xmm1, %xmm10
	.insn EVEX.LIG.F3.0F.W0 0x58, 4(%rax){:d4}, %xmm1, %xmm2

	# vfmaddps
	.insn VEX.66.0F3A.W0 0x68, %xmm8, (%rcx), %xmm2, %xmm3
	.insn VEX.66.0F3A.W1 0x68, %xmm0, (%ecx), %xmm2, %xmm3
	.insn VEX.66.0F3A.W1 0x68, (%r8), %xmm1, %xmm2, %xmm3

	# vpermil2ps
	.insn VEX.66.0F3A.W0 0x48, $0, %xmm8, (%rcx), %xmm2, %xmm3
	.insn VEX.66.0F3A.W1 0x48, $2, %xmm0, (%ecx), %xmm2, %xmm3
	.insn VEX.66.0F3A.W1 0x48, $3, (%r8), %xmm1, %xmm2, %xmm3

	# kmovw
	.insn VEX.L0.0F.W0 0x92, %r8d, %k1
	.insn VEX.L0.0F.W0 0x93, %k1, %r8d

	# vaddps
	.insn EVEX.NP.0F.W0 0x58, {rd-sae}, %zmm16, %zmm1, %zmm2
	.insn EVEX.NP.0F.W0 0x58, {rn-sae}, %zmm0, %zmm17, %zmm2
	.insn EVEX.NP.0F.W0 0x58, {ru-sae}, %zmm0, %zmm1, %zmm18

	# vgather...
	.insn VEX.66.0f38.W0 0x92, %xmm8, (%rax, %xmm1, 2), %xmm3
	.insn VEX.66.0f38.W0 0x92, %xmm0, (%r8, %xmm1, 2), %xmm3
	.insn VEX.66.0f38.W0 0x92, %xmm0, (%rax, %xmm9, 2), %xmm3
	.insn VEX.66.0f38.W0 0x92, %xmm0, (%rax, %xmm1, 2), %xmm11
	.insn EVEX.66.0f38.W1 0x93, (%r8, %xmm1, 2), %xmm3{%k4}
	.insn EVEX.66.0f38.W1 0x93, (%rax, %xmm9, 2), %xmm3{%k4}
	.insn EVEX.66.0f38.W1 0x93, (%rax, %xmm17, 2), %xmm3{%k4}
	.insn EVEX.66.0f38.W1 0x93, (%rax, %xmm1, 2), %xmm11{%k4}
	.insn EVEX.66.0f38.W1 0x93, (%rax, %xmm1, 2), %xmm19{%k4}

	# vexpandps
	.insn EVEX.66.0F38.W0 0x88, 4(%rax){:d4}, %ymm1

	# vcvtpd2phz
	.insn EVEX.512.66.M5.W1 0x5a, 64(%rax), %xmm0
	.insn EVEX.66.M5.W1 0x5a, 64(%rax), %zmm0
	.insn EVEX.66.M5.W1 0x5a, 64(%rax){:d64}, %xmm0
	.insn EVEX.512.66.M5.W1 0x5a, 8(%rax){1to8}, %xmm0
	.insn EVEX.66.M5.W1 0x5a, 8(%rax){1to8}, %zmm0
	.insn EVEX.66.M5.W1 0x5a, 8(%rax){1to8:d8}, %xmm0

	# vcvtph2pd
	.insn EVEX.M5.W0 0x5a, 16(%rax){:d16}, %zmm0
	.insn EVEX.M5.W0 0x5a, 2(%rax){1to8:d2}, %zmm0
