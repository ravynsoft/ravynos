# Check VPCLMULQDQ instructions

	.allow_index_reg
	.text
_start:
	vpclmulqdq	$0xab, %ymm4, %ymm5, %ymm6
	vpclmulqdq	$123, -123456(%esp,%esi,8), %ymm5, %ymm6
	vpclmulqdq	$123, 4064(%edx), %ymm5, %ymm6

	vpclmulhqhqdq	%ymm1, %ymm2, %ymm3
	vpclmulhqlqdq	%ymm2, %ymm3, %ymm4
	vpclmullqhqdq	%ymm3, %ymm4, %ymm5
	vpclmullqlqdq	%ymm4, %ymm5, %ymm6

	.intel_syntax noprefix
	vpclmulqdq	ymm6, ymm5, ymm4, 0xab
	vpclmulqdq	ymm6, ymm5, YMMWORD PTR [esp+esi*8-123456], 123
	vpclmulqdq	ymm6, ymm5, YMMWORD PTR [edx+4064], 123
