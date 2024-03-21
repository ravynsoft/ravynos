# Check 32bit AVX512F,VPCLMULQDQ instructions

	.allow_index_reg
	.text
_start:
	vpclmulqdq	$0xab, %zmm1, %zmm3, %zmm1	 # AVX512F,VPCLMULQDQ
	vpclmulqdq	$123, -123456(%esp,%esi,8), %zmm3, %zmm1	 # AVX512F,VPCLMULQDQ
	vpclmulqdq	$123, 8128(%edx), %zmm3, %zmm1	 # AVX512F,VPCLMULQDQ Disp8

	vpclmulhqhqdq	%zmm1, %zmm2, %zmm3
	vpclmulhqlqdq	%zmm2, %zmm3, %zmm4
	vpclmullqhqdq	%zmm3, %zmm4, %zmm5
	vpclmullqlqdq	%zmm4, %zmm5, %zmm6

	.intel_syntax noprefix
	vpclmulqdq	zmm2, zmm2, zmm2, 0xab	 # AVX512F,VPCLMULQDQ
	vpclmulqdq	zmm2, zmm2, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F,VPCLMULQDQ
	vpclmulqdq	zmm2, zmm2, ZMMWORD PTR [edx+8128], 123	 # AVX512F,VPCLMULQDQ Disp8
