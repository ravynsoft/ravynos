# Check 64bit instructions with optimized encoding

	.arch .avx512vl

.include "x86-64-optimize-2.s"

	{evex} vandnpd %zmm1, %zmm1, %zmm5
	{evex} vandnpd %ymm1, %ymm1, %ymm5

	{evex} vmovdqa32	%ymm1, %ymm2
	{evex} vmovdqa64	%ymm1, %ymm2
	{evex} vmovdqu8		%xmm1, %xmm2
	{evex} vmovdqu16	%xmm1, %xmm2
	{evex} vmovdqu32	%xmm1, %xmm2
	{evex} vmovdqu64	%xmm1, %xmm2

	{evex} vpandd	%xmm2, %xmm3, %xmm4
	{evex} vpandq	%ymm12, %ymm3, %ymm4
	{evex} vpandnd	%ymm2, %ymm13, %ymm4
	{evex} vpandnq	%xmm2, %xmm3, %xmm14
	{evex} vpord	%xmm2, %xmm3, %xmm4
	{evex} vporq	%ymm12, %ymm3, %ymm4
	{evex} vpxord	%ymm2, %ymm13, %ymm4
	{evex} vpxorq	%xmm2, %xmm3, %xmm14
