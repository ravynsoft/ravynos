	.text
	.arch corei7
_start:
	.arch .avx
	vmovntdq	%xmm2, (%ecx)
	vmovntdq	%ymm2, (%ecx)
	vmovntdq	%zmm2, (%ecx)
	vpternlogq	$0xab, %xmm6, %xmm2, %xmm0
	vpternlogq	$0xab, %ymm6, %ymm2, %ymm0
	vpternlogq	$0xab, %zmm6, %zmm2, %zmm0
	.arch .avx512f
	vmovntdq	%xmm0, (%ecx)
	vmovntdq	%ymm0, (%ecx)
	vmovntdq	%zmm0, (%ecx)
	vpternlogq	$0xab, %xmm6, %xmm2, %xmm0
	vpternlogq	$0xab, %ymm6, %ymm2, %ymm0
	vpternlogq	$0xab, %zmm6, %zmm2, %zmm0
	.arch .avx512vl
	vmovntdq	%xmm0, (%ecx)
	vmovntdq	%ymm0, (%ecx)
	vmovntdq	%zmm0, (%ecx)
	vpternlogq	$0xab, %xmm6, %xmm2, %xmm0
	vpternlogq	$0xab, %ymm6, %ymm2, %ymm0
	vpternlogq	$0xab, %zmm6, %zmm2, %zmm0
