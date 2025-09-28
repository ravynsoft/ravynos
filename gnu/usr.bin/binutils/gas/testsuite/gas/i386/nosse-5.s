# Test .arch .nosse with .noavx/.avx/.avx512vl
	.text
	.arch generic32
	.arch .avx
	vaddps	%xmm6, %xmm5, %xmm4
	vaddps	%ymm6, %ymm5, %ymm4
	vaddss	%xmm6, %xmm5, %xmm4
	.arch .nosse
	vaddps	%xmm6, %xmm5, %xmm4
	vaddps	%ymm6, %ymm5, %ymm4
	vaddss	%xmm6, %xmm5, %xmm4
	.arch .avx512vl
	vaddps	%xmm6, %xmm5, %xmm4{%k7}
	vaddps	%ymm6, %ymm5, %ymm4{%k7}
	vaddps	%zmm6, %zmm5, %zmm4
	.arch .nosse
	vaddps	%xmm6, %xmm5, %xmm4{%k7}
	vaddps	%ymm6, %ymm5, %ymm4{%k7}
	vaddps	%zmm6, %zmm5, %zmm4
	vaddps	%xmm6, %xmm5, %xmm4
	vaddps	%ymm6, %ymm5, %ymm4
	vaddss	%xmm6, %xmm5, %xmm4
