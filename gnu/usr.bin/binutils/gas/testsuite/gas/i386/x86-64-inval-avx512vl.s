# Check illegal AVX512VL instructions
	.text
	.arch .noavx512vl
_start:
	vp2intersectd 32(%rax), %ymm2, %k3
	vp2intersectq 16(%rax), %xmm2, %k3
