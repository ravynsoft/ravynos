

	.allow_index_reg
	.text
_start:
	vpclmulqdq	$0xab, %ymm8, %ymm9, %ymm10
	vpclmulqdq	$123, 0x124(%rax,%r14,8), %ymm9, %ymm10
	vpclmulqdq	$123, 4064(%rdx), %ymm9, %ymm10

	vpclmulhqhqdq	%ymm10, %ymm11, %ymm12
	vpclmulhqlqdq	%ymm11, %ymm12, %ymm13
	vpclmullqhqdq	%ymm12, %ymm13, %ymm14
	vpclmullqlqdq	%ymm13, %ymm14, %ymm15

	.intel_syntax noprefix
	vpclmulqdq	ymm10, ymm9, ymm8, 0xab
	vpclmulqdq	ymm10, ymm9, YMMWORD PTR [rax+r14*8+0x1234], 123
	vpclmulqdq	ymm10, ymm9, YMMWORD PTR [rdx+4064], 123
