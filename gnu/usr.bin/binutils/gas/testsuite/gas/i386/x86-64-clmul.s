# Check 64bit PCLMUL new instructions.

	.text
foo:
	pclmulqdq $8,(%rcx),%xmm0
	pclmulqdq $8,%xmm1,%xmm0
	pclmullqlqdq (%rcx),%xmm0
	pclmullqlqdq %xmm1,%xmm0
	pclmulhqlqdq (%rcx),%xmm0
	pclmulhqlqdq %xmm1,%xmm0
	pclmullqhqdq (%rcx),%xmm0
	pclmullqhqdq %xmm1,%xmm0
	pclmulhqhqdq (%rcx),%xmm0
	pclmulhqhqdq %xmm1,%xmm0

	.intel_syntax noprefix
	pclmulqdq xmm0,XMMWORD PTR [rcx],8
	pclmulqdq xmm0,xmm1,8
	pclmullqlqdq xmm0,XMMWORD PTR [rcx]
	pclmullqlqdq xmm0,xmm1
	pclmulhqlqdq xmm0,XMMWORD PTR [rcx]
	pclmulhqlqdq xmm0,xmm1
	pclmullqhqdq xmm0,XMMWORD PTR [rcx]
	pclmullqhqdq xmm0,xmm1
	pclmulhqhqdq xmm0,XMMWORD PTR [rcx]
	pclmulhqhqdq xmm0,xmm1
