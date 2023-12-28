# Check instructions with optimized encoding

	.arch .avx512vl

.include "optimize-1.s"

	{evex} vandnpd %zmm1, %zmm1, %zmm5
	{evex} vandnpd %ymm1, %ymm1, %ymm5

	{evex} vmovdqa32	%ymm1, %ymm2
	{evex} vmovdqa64	%ymm1, %ymm2
	{evex} vmovdqu8		%xmm1, %xmm2
	{evex} vmovdqu16	%xmm1, %xmm2
	{evex} vmovdqu32	%xmm1, %xmm2
	{evex} vmovdqu64	%xmm1, %xmm2

	{evex} vpandd	%xmm2, %xmm3, %xmm4
	{evex} vpandq	%ymm2, %ymm3, %ymm4
	{evex} vpandnd	%ymm2, %ymm3, %ymm4
	{evex} vpandnq	%xmm2, %xmm3, %xmm4
	{evex} vpord	%xmm2, %xmm3, %xmm4
	{evex} vporq	%ymm2, %ymm3, %ymm4
	{evex} vpxord	%ymm2, %ymm3, %ymm4
	{evex} vpxorq	%xmm2, %xmm3, %xmm4
