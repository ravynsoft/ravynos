	.text
	.arch generic32
	.arch .avx512f
_start:
	vaesenc		%zmm1, %zmm2, %zmm3
	vgf2p8mulb	%zmm1, %zmm2, %zmm3
	vpclmulqdq	$0, %zmm1, %zmm2, %zmm3

	.arch .vaes
	vaesenc		%zmm1, %zmm2, %zmm3

	.arch .gfni
	vgf2p8mulb	%zmm1, %zmm2, %zmm3

	.arch .vpclmulqdq
	vpclmulqdq	$0, %zmm1, %zmm2, %zmm3
	.p2align 4,0
