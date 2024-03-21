# Check 64bit AES new instructions.

	.text
foo:
	aesenc	(%rcx),%xmm0
	aesenc	%xmm1,%xmm0
	aesenclast	(%rcx),%xmm0
	aesenclast	%xmm1,%xmm0
	aesdec	(%rcx),%xmm0
	aesdec	%xmm1,%xmm0
	aesdeclast	(%rcx),%xmm0
	aesdeclast	%xmm1,%xmm0
	aesimc	(%rcx),%xmm0
	aesimc	%xmm1,%xmm0
	aeskeygenassist	$8,(%rcx),%xmm0
	aeskeygenassist	$8,%xmm1,%xmm0

	.intel_syntax noprefix
	aesenc xmm0,XMMWORD PTR [rcx]
	aesenc xmm0,xmm1
	aesenclast xmm0,XMMWORD PTR [rcx]
	aesenclast xmm0,xmm1
	aesdec xmm0,XMMWORD PTR [rcx]
	aesdec xmm0,xmm1
	aesdeclast xmm0,XMMWORD PTR [rcx]
	aesdeclast xmm0,xmm1
	aesimc xmm0,XMMWORD PTR [rcx]
	aesimc xmm0,xmm1
	aeskeygenassist xmm0,XMMWORD PTR [rcx],8
	aeskeygenassist xmm0,xmm1,8
