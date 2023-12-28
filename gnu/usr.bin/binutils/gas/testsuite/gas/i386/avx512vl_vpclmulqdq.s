# Check 32bit AVX512VL,VPCLMULQDQ instructions

	.allow_index_reg
	.text
_start:
	vpclmulqdq	$0xab, %xmm2, %xmm2, %xmm3	 # AVX512VL,VPCLMULQDQ
	vpclmulqdq	$123, -123456(%esp,%esi,8), %xmm2, %xmm3	 # AVX512VL,VPCLMULQDQ
	vpclmulqdq	$123, 2032(%edx), %xmm2, %xmm3	 # AVX512VL,VPCLMULQDQ Disp8
	vpclmulqdq	$0xab, %ymm1, %ymm5, %ymm4	 # AVX512VL,VPCLMULQDQ
	vpclmulqdq	$123, -123456(%esp,%esi,8), %ymm5, %ymm4	 # AVX512VL,VPCLMULQDQ
	vpclmulqdq	$123, 4064(%edx), %ymm5, %ymm4	 # AVX512VL,VPCLMULQDQ Disp8

	{evex} vpclmulqdq	$0xab, %xmm2, %xmm2, %xmm3	 # AVX512VL,VPCLMULQDQ
	{evex} vpclmulqdq	$123, -123456(%esp,%esi,8), %xmm2, %xmm3	 # AVX512VL,VPCLMULQDQ
	{evex} vpclmulqdq	$123, 2032(%edx), %xmm2, %xmm3	 # AVX512VL,VPCLMULQDQ Disp8
	{evex} vpclmulqdq	$0xab, %ymm1, %ymm5, %ymm4	 # AVX512VL,VPCLMULQDQ
	{evex} vpclmulqdq	$123, -123456(%esp,%esi,8), %ymm5, %ymm4	 # AVX512VL,VPCLMULQDQ
	{evex} vpclmulqdq	$123, 4064(%edx), %ymm5, %ymm4	 # AVX512VL,VPCLMULQDQ Disp8

	{evex} vpclmulhqhqdq	%xmm2, %xmm3, %xmm4
	{evex} vpclmulhqlqdq	%xmm3, %xmm4, %xmm5
	{evex} vpclmullqhqdq	%xmm4, %xmm5, %xmm6
	{evex} vpclmullqlqdq	%xmm5, %xmm6, %xmm7

	{evex} vpclmulhqhqdq	%ymm1, %ymm2, %ymm3
	{evex} vpclmulhqlqdq	%ymm2, %ymm3, %ymm4
	{evex} vpclmullqhqdq	%ymm3, %ymm4, %ymm5
	{evex} vpclmullqlqdq	%ymm4, %ymm5, %ymm6

	.intel_syntax noprefix
	vpclmulqdq	xmm3, xmm5, xmm3, 0xab	 # AVX512VL,VPCLMULQDQ
	vpclmulqdq	xmm3, xmm5, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512VL,VPCLMULQDQ
	vpclmulqdq	xmm3, xmm5, XMMWORD PTR [edx+2032], 123	 # AVX512VL,VPCLMULQDQ Disp8
	vpclmulqdq	ymm2, ymm2, ymm2, 0xab	 # AVX512VL,VPCLMULQDQ
	vpclmulqdq	ymm2, ymm2, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512VL,VPCLMULQDQ
	vpclmulqdq	ymm2, ymm2, YMMWORD PTR [edx+4064], 123	 # AVX512VL,VPCLMULQDQ Disp8

	{evex} vpclmulqdq	xmm3, xmm5, xmm3, 0xab	 # AVX512VL,VPCLMULQDQ
	{evex} vpclmulqdq	xmm3, xmm5, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512VL,VPCLMULQDQ
	{evex} vpclmulqdq	xmm3, xmm5, XMMWORD PTR [edx+2032], 123	 # AVX512VL,VPCLMULQDQ Disp8
	{evex} vpclmulqdq	ymm2, ymm2, ymm2, 0xab	 # AVX512VL,VPCLMULQDQ
	{evex} vpclmulqdq	ymm2, ymm2, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512VL,VPCLMULQDQ
	{evex} vpclmulqdq	ymm2, ymm2, YMMWORD PTR [edx+4064], 123	 # AVX512VL,VPCLMULQDQ Disp8
