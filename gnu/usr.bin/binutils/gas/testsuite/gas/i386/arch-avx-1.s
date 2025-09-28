# Test -march=
	.text
# AES + AVX
vaesenc  (%ecx),%xmm0,%xmm2
# PCLMUL + AVX
vpclmulqdq $8,%xmm4,%xmm6,%xmm2
# GFNI + AVX
vgf2p8mulb %xmm1,%xmm2,%xmm3
