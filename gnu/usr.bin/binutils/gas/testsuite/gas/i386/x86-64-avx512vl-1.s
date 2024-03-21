	.text
	.arch corei7
_start:
	.arch .avx
	vmovntdq	%xmm2, (%rcx)
	vmovntdq	%ymm2, (%rcx)
	vmovntdq	%zmm2, (%rcx)
	vpternlogq	$0xab, %xmm16, %xmm2, %xmm0
	vpternlogq	$0xab, %ymm16, %ymm2, %ymm0
	vpternlogq	$0xab, %zmm16, %zmm2, %zmm0
	.arch .avx512f
	vmovntdq	%xmm0, (%rcx)
	vmovntdq	%ymm0, (%rcx)
	vmovntdq	%zmm0, (%rcx)
	vpternlogq	$0xab, %xmm16, %xmm2, %xmm0
	vpternlogq	$0xab, %ymm16, %ymm2, %ymm0
	vpternlogq	$0xab, %zmm16, %zmm2, %zmm0
	.arch .avx512vl
	vmovntdq	%xmm0, (%rcx)
	vmovntdq	%ymm0, (%rcx)
	vmovntdq	%zmm0, (%rcx)
	vpternlogq	$0xab, %xmm16, %xmm2, %xmm0
	vpternlogq	$0xab, %ymm16, %ymm2, %ymm0
	vpternlogq	$0xab, %zmm16, %zmm2, %zmm0
