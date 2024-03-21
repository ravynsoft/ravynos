# Check AVX512_VP2INTERSECT new instructions.

	.text
	vp2intersectd %zmm1, %zmm2, %k3
	vp2intersectd 64(%rax), %zmm2, %k3
	vp2intersectd 8(%rax){1to16}, %zmm2, %k3

	vp2intersectd %ymm1, %ymm2, %k3
	vp2intersectd 32(%rax), %ymm2, %k3
	vp2intersectd 8(%rax){1to8}, %ymm2, %k3

	vp2intersectd %xmm1, %xmm2, %k3
	vp2intersectd 16(%rax), %xmm2, %k3
	vp2intersectd 8(%rax){1to4}, %xmm2, %k3

	vp2intersectq %zmm1, %zmm2, %k3
	vp2intersectq 64(%rax), %zmm2, %k3
	vp2intersectq 8(%rax){1to8}, %zmm2, %k3

	vp2intersectq %ymm1, %ymm2, %k3
	vp2intersectq 32(%rax), %ymm2, %k3
	vp2intersectq 8(%rax){1to4}, %ymm2, %k3

	vp2intersectq %xmm1, %xmm2, %k3
	vp2intersectq 16(%rax), %xmm2, %k3
	vp2intersectq 8(%rax){1to2}, %xmm2, %k3

	.intel_syntax noprefix
	vp2intersectd k3, zmm2, zmm1
	vp2intersectd k3, zmm2, 64[rax]
	vp2intersectd k3, zmm2, dword bcst 8[rax]

	vp2intersectd k3, ymm2, ymm1
	vp2intersectd k3, ymm2, 32[rax]
	vp2intersectd k3, ymm2, dword bcst 8[rax]

	vp2intersectd k3, xmm2, xmm1
	vp2intersectd k3, xmm2, 16[rax]
	vp2intersectd k3, xmm2, dword bcst 8[rax]

	vp2intersectq k3, zmm2, zmm1
	vp2intersectq k3, zmm2, 64[rax]
	vp2intersectq k3, zmm2, qword bcst 8[rax]

	vp2intersectq k3, ymm2, ymm1
	vp2intersectq k3, ymm2, 32[rax]
	vp2intersectq k3, ymm2, qword bcst 8[rax]

	vp2intersectq k3, xmm2, xmm1
	vp2intersectq k3, xmm2, 16[rax]
	vp2intersectq k3, xmm2, qword bcst 8[rax]
