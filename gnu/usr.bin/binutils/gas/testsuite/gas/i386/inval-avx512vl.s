# Check illegal AVX512VL instructions
	.text
	.arch .noavx512vl
_start:
	vp2intersectd %ymm1, %ymm2, %k3
	vp2intersectq %ymm1, %ymm2, %k3
