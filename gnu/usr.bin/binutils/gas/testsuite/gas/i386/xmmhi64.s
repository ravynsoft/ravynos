	.text
	.intel_syntax noprefix
	.code64
xmm:
	vaddps	xmm0, xmm1, xmm16
	vaddps	ymm0, ymm1, ymm16
	vaddps	zmm0, zmm1, zmm16
