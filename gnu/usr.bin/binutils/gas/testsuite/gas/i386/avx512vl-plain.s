	.text
	.arch generic32
	.arch .avx512vl
_start:
	{evex} vaesenc		%ymm1, %ymm2, %ymm3
	       vgf2p8mulb	%ymm1, %ymm2, %ymm3{%k4}
	{evex} vpclmulqdq	$0, %ymm1, %ymm2, %ymm3

	.arch .vaes
	{evex} vaesenc		%ymm1, %ymm2, %ymm3

	.arch .gfni
	       vgf2p8mulb	%ymm1, %ymm2, %ymm3{%k4}

	.arch .vpclmulqdq
	{evex} vpclmulqdq	$0, %ymm1, %ymm2, %ymm3
	.p2align 4,0
