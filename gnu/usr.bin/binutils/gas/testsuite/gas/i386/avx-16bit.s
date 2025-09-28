# Check AVX instructions in 16-bit mode

	.code16
	.include "avx.s"
	.att_syntax prefix

	vaddps (%bx),%ymm6,%ymm2
