# Check 64bit AVX512VL,VPCLMULQDQ instructions

	.allow_index_reg
	.text
_start:
	vpclmulqdq	$0xab, %xmm18, %xmm29, %xmm25	 # AVX512VL,VPCLMULQDQ
	vpclmulqdq	$123, 0x123(%rax,%r14,8), %xmm29, %xmm25	 # AVX512VL,VPCLMULQDQ
	vpclmulqdq	$123, 2032(%rdx), %xmm29, %xmm25	 # AVX512VL,VPCLMULQDQ Disp8
	vpclmulqdq	$0xab, %ymm18, %ymm18, %ymm29	 # AVX512VL,VPCLMULQDQ
	vpclmulqdq	$123, 0x123(%rax,%r14,8), %ymm18, %ymm29	 # AVX512VL,VPCLMULQDQ
	vpclmulqdq	$123, 4064(%rdx), %ymm18, %ymm29	 # AVX512VL,VPCLMULQDQ Disp8

	{evex} vpclmulqdq	$0xab, %xmm18, %xmm29, %xmm25	 # AVX512VL,VPCLMULQDQ
	{evex} vpclmulqdq	$123, 0x123(%rax,%r14,8), %xmm29, %xmm25	 # AVX512VL,VPCLMULQDQ
	{evex} vpclmulqdq	$123, 2032(%rdx), %xmm29, %xmm25	 # AVX512VL,VPCLMULQDQ Disp8
	{evex} vpclmulqdq	$0xab, %ymm18, %ymm18, %ymm29	 # AVX512VL,VPCLMULQDQ
	{evex} vpclmulqdq	$123, 0x123(%rax,%r14,8), %ymm18, %ymm29	 # AVX512VL,VPCLMULQDQ
	{evex} vpclmulqdq	$123, 4064(%rdx), %ymm18, %ymm29	 # AVX512VL,VPCLMULQDQ Disp8

	vpclmulhqhqdq	%xmm20, %xmm21, %xmm22
	vpclmulhqlqdq	%xmm21, %xmm22, %xmm23
	vpclmullqhqdq	%xmm22, %xmm23, %xmm24
	vpclmullqlqdq	%xmm23, %xmm24, %xmm25

	vpclmulhqhqdq	%ymm20, %ymm21, %ymm22
	vpclmulhqlqdq	%ymm21, %ymm22, %ymm23
	vpclmullqhqdq	%ymm22, %ymm23, %ymm24
	vpclmullqlqdq	%ymm23, %ymm24, %ymm25

	.intel_syntax noprefix
	vpclmulqdq	xmm19, xmm26, xmm20, 0xab	 # AVX512VL,VPCLMULQDQ
	vpclmulqdq	xmm19, xmm26, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512VL,VPCLMULQDQ
	vpclmulqdq	xmm19, xmm26, XMMWORD PTR [rdx+2032], 123	 # AVX512VL,VPCLMULQDQ Disp8
	vpclmulqdq	ymm23, ymm29, ymm27, 0xab	 # AVX512VL,VPCLMULQDQ
	vpclmulqdq	ymm23, ymm29, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512VL,VPCLMULQDQ
	vpclmulqdq	ymm23, ymm29, YMMWORD PTR [rdx+4064], 123	 # AVX512VL,VPCLMULQDQ Disp8

	{evex} vpclmulqdq	xmm19, xmm26, xmm20, 0xab	 # AVX512VL,VPCLMULQDQ
	{evex} vpclmulqdq	xmm19, xmm26, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512VL,VPCLMULQDQ
	{evex} vpclmulqdq	xmm19, xmm26, XMMWORD PTR [rdx+2032], 123	 # AVX512VL,VPCLMULQDQ Disp8
	{evex} vpclmulqdq	ymm23, ymm29, ymm27, 0xab	 # AVX512VL,VPCLMULQDQ
	{evex} vpclmulqdq	ymm23, ymm29, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512VL,VPCLMULQDQ
	{evex} vpclmulqdq	ymm23, ymm29, YMMWORD PTR [rdx+4064], 123	 # AVX512VL,VPCLMULQDQ Disp8
