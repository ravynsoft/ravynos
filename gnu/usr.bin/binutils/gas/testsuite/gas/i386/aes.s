# Check AES new instructions.

	.text
foo:
	aesenc	(%ecx),%xmm0
	aesenc	%xmm1,%xmm0
	aesenclast	(%ecx),%xmm0
	aesenclast	%xmm1,%xmm0
	aesdec	(%ecx),%xmm0
	aesdec	%xmm1,%xmm0
	aesdeclast	(%ecx),%xmm0
	aesdeclast	%xmm1,%xmm0
	aesimc	(%ecx),%xmm0
	aesimc	%xmm1,%xmm0
	aeskeygenassist	$8,(%ecx),%xmm0
	aeskeygenassist	$8,%xmm1,%xmm0

	.intel_syntax noprefix
	aesenc xmm0,XMMWORD PTR [ecx]
	aesenc xmm0,xmm1
	aesenclast xmm0,XMMWORD PTR [ecx]
	aesenclast xmm0,xmm1
	aesdec xmm0,XMMWORD PTR [ecx]
	aesdec xmm0,xmm1
	aesdeclast xmm0,XMMWORD PTR [ecx]
	aesdeclast xmm0,xmm1
	aesimc xmm0,XMMWORD PTR [ecx]
	aesimc xmm0,xmm1
	aeskeygenassist xmm0,XMMWORD PTR [ecx],8
	aeskeygenassist xmm0,xmm1,8
