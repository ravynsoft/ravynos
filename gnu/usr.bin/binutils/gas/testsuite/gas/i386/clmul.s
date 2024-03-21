# Check PCLMUL new instructions.

	.text
foo:
	pclmulqdq $8,(%ecx),%xmm0
	pclmulqdq $8,%xmm1,%xmm0
	pclmullqlqdq (%ecx),%xmm0
	pclmullqlqdq %xmm1,%xmm0
	pclmulhqlqdq (%ecx),%xmm0
	pclmulhqlqdq %xmm1,%xmm0
	pclmullqhqdq (%ecx),%xmm0
	pclmullqhqdq %xmm1,%xmm0
	pclmulhqhqdq (%ecx),%xmm0
	pclmulhqhqdq %xmm1,%xmm0

	.intel_syntax noprefix
	pclmulqdq xmm0,XMMWORD PTR [ecx],8
	pclmulqdq xmm0,xmm1,8
	pclmullqlqdq xmm0,XMMWORD PTR [ecx]
	pclmullqlqdq xmm0,xmm1
	pclmulhqlqdq xmm0,XMMWORD PTR [ecx]
	pclmulhqlqdq xmm0,xmm1
	pclmullqhqdq xmm0,XMMWORD PTR [ecx]
	pclmullqhqdq xmm0,xmm1
	pclmulhqhqdq xmm0,XMMWORD PTR [ecx]
	pclmulhqhqdq xmm0,xmm1
