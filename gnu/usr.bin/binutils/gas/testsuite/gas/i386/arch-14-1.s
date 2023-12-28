# Test -march=
	.text

	vaddpd	%zmm4, %zmm5, %zmm6	 # AVX512F
	vbroadcastf32x8	(%ecx), %zmm6	 # AVX512DQ
	vpmadd52luq	%zmm4, %zmm5, %zmm6	 # AVX512IFMA
	vpconflictd	%zmm5, %zmm6	 # AVX512CD
	vpabsb	%zmm5, %zmm6	 # AVX512BW
	vaddpd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{F,VL}
	vcvtne2ps2bf16	%zmm4, %zmm5, %zmm6	 #AVX512_BF16
	vpermb	%zmm4, %zmm5, %zmm6	 # AVX512VBMI
	vpcompressb	%zmm6, (%ecx){%k7}	 # AVX512VBMI2
	vpdpwssd	%zmm3, %zmm1, %zmm4	 # AVX512VNNI
	vpshufbitqmb	%zmm4, %zmm5, %k5	 # AVX512BITALG
	vpopcntd	%zmm5, %zmm6	 # AVX512_VPOPCNTDQ
	gf2p8mulb %xmm4, %xmm5	 # GFNI
