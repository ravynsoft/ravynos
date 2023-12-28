# Check instructions with optimized encoding

	.allow_index_reg
	.text
_start:
	{nooptimize} testl $0x7f, %eax

	{nooptimize} lock xchg	%ecx, (%edx)
	{nooptimize} lock xchg	(%ecx), %edx

	{nooptimize} vmovdqa32	%ymm1, %ymm2
	{nooptimize} vmovdqa64	%ymm1, %ymm2
	{nooptimize} vmovdqu8	%xmm1, %xmm2
	{nooptimize} vmovdqu16	%xmm1, %xmm2
	{nooptimize} vmovdqu32	%xmm1, %xmm2
	{nooptimize} vmovdqu64	%xmm1, %xmm2

	{nooptimize} vpandd	%xmm2, %xmm3, %xmm4
	{nooptimize} vpandq	%ymm2, %ymm3, %ymm4
	{nooptimize} vpandnd	%ymm2, %ymm3, %ymm4
	{nooptimize} vpandnq	%xmm2, %xmm3, %xmm4
	{nooptimize} vpord	%xmm2, %xmm3, %xmm4
	{nooptimize} vporq	%ymm2, %ymm3, %ymm4
	{nooptimize} vpxord	%ymm2, %ymm3, %ymm4
	{nooptimize} vpxorq	%xmm2, %xmm3, %xmm4
