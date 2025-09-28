# Test -march=
	.text

	vaddpd	%zmm28, %zmm29, %zmm30	 # AVX512F
	vbroadcastf32x8	(%rcx), %zmm30	 # AVX512DQ
	vpmadd52luq	%zmm28, %zmm29, %zmm30	 # AVX512IFMA
	vpconflictd	%zmm29, %zmm30	 # AVX512CD
	vpabsb	%zmm29, %zmm30	 # AVX512BW
	vaddpd	%xmm28, %xmm29, %xmm30	 # AVX512{F,VL}
	vcvtne2ps2bf16	%zmm28, %zmm29, %zmm30	 #AVX512_BF16
	vpermb	%zmm28, %zmm29, %zmm30	 # AVX512VBMI
	vpcompressb	%zmm30, (%rcx){%k7}	 # AVX512VBMI2
	vpdpwssd	%zmm17, %zmm18, %zmm18	 # AVX512VNNI
	vpshufbitqmb	%zmm28, %zmm29, %k5	 # AVX512BITALG
	vpopcntd	%zmm29, %zmm30	 # AVX512_VPOPCNTDQ
	gf2p8mulb %xmm4, %xmm5
# RMPQUERY
	rmpquery
