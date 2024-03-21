# Check EVEX WIG instructions

	.allow_index_reg
	.text
_start:

	vextractps	$0xab, %xmm29, %rax	 # AVX512
	vextractps	$123, %xmm29, %rax	 # AVX512
	vextractps	$123, %xmm29, %r8	 # AVX512
	vextractps	$123, %xmm29, (%rcx)	 # AVX512
	vextractps	$123, %xmm29, 0x123(%rax,%r14,8)	 # AVX512
	vextractps	$123, %xmm29, 508(%rdx)	 # AVX512 Disp8
	vextractps	$123, %xmm29, 512(%rdx)	 # AVX512
	vextractps	$123, %xmm29, -512(%rdx)	 # AVX512 Disp8
	vextractps	$123, %xmm29, -516(%rdx)	 # AVX512

	{evex} vpextrb $0, %xmm0, %eax
	{evex} vpextrb $0, %xmm0, (%rax)

	{evex} vpextrw $0, %xmm0, %eax
	{evex} {store} vpextrw $0, %xmm0, %eax
	{evex} vpextrw $0, %xmm0, (%rax)

	{evex} vpinsrb $0, %eax, %xmm0, %xmm0
	{evex} vpinsrb $0, (%rax), %xmm0, %xmm0

	{evex} vpinsrw $0, %eax, %xmm0, %xmm0
	{evex} vpinsrw $0, (%rax), %xmm0, %xmm0

	vpmovsxbd	%xmm29, %zmm30{%k7}	 # AVX512
	vpmovsxbd	%xmm29, %zmm30{%k7}{z}	 # AVX512
	vpmovsxbd	(%rcx), %zmm30{%k7}	 # AVX512
	vpmovsxbd	0x123(%rax,%r14,8), %zmm30{%k7}	 # AVX512
	vpmovsxbd	2032(%rdx), %zmm30{%k7}	 # AVX512 Disp8
	vpmovsxbd	2048(%rdx), %zmm30{%k7}	 # AVX512
	vpmovsxbd	-2048(%rdx), %zmm30{%k7}	 # AVX512 Disp8
	vpmovsxbd	-2064(%rdx), %zmm30{%k7}	 # AVX512

	vpmovsxbq	%xmm29, %zmm30{%k7}	 # AVX512
	vpmovsxbq	%xmm29, %zmm30{%k7}{z}	 # AVX512
	vpmovsxbq	(%rcx), %zmm30{%k7}	 # AVX512
	vpmovsxbq	0x123(%rax,%r14,8), %zmm30{%k7}	 # AVX512
	vpmovsxbq	1016(%rdx), %zmm30{%k7}	 # AVX512 Disp8
	vpmovsxbq	1024(%rdx), %zmm30{%k7}	 # AVX512
	vpmovsxbq	-1024(%rdx), %zmm30{%k7}	 # AVX512 Disp8
	vpmovsxbq	-1032(%rdx), %zmm30{%k7}	 # AVX512

	vpmovsxwd	%ymm29, %zmm30{%k7}	 # AVX512
	vpmovsxwd	%ymm29, %zmm30{%k7}{z}	 # AVX512
	vpmovsxwd	(%rcx), %zmm30{%k7}	 # AVX512
	vpmovsxwd	0x123(%rax,%r14,8), %zmm30{%k7}	 # AVX512
	vpmovsxwd	4064(%rdx), %zmm30{%k7}	 # AVX512 Disp8
	vpmovsxwd	4096(%rdx), %zmm30{%k7}	 # AVX512
	vpmovsxwd	-4096(%rdx), %zmm30{%k7}	 # AVX512 Disp8
	vpmovsxwd	-4128(%rdx), %zmm30{%k7}	 # AVX512

	vpmovsxwq	%xmm29, %zmm30{%k7}	 # AVX512
	vpmovsxwq	%xmm29, %zmm30{%k7}{z}	 # AVX512
	vpmovsxwq	(%rcx), %zmm30{%k7}	 # AVX512
	vpmovsxwq	0x123(%rax,%r14,8), %zmm30{%k7}	 # AVX512
	vpmovsxwq	2032(%rdx), %zmm30{%k7}	 # AVX512 Disp8
	vpmovsxwq	2048(%rdx), %zmm30{%k7}	 # AVX512
	vpmovsxwq	-2048(%rdx), %zmm30{%k7}	 # AVX512 Disp8
	vpmovsxwq	-2064(%rdx), %zmm30{%k7}	 # AVX512

	vpmovzxbd	%xmm29, %zmm30{%k7}	 # AVX512
	vpmovzxbd	%xmm29, %zmm30{%k7}{z}	 # AVX512
	vpmovzxbd	(%rcx), %zmm30{%k7}	 # AVX512
	vpmovzxbd	0x123(%rax,%r14,8), %zmm30{%k7}	 # AVX512
	vpmovzxbd	2032(%rdx), %zmm30{%k7}	 # AVX512 Disp8
	vpmovzxbd	2048(%rdx), %zmm30{%k7}	 # AVX512
	vpmovzxbd	-2048(%rdx), %zmm30{%k7}	 # AVX512 Disp8
	vpmovzxbd	-2064(%rdx), %zmm30{%k7}	 # AVX512

	vpmovzxbq	%xmm29, %zmm30{%k7}	 # AVX512
	vpmovzxbq	%xmm29, %zmm30{%k7}{z}	 # AVX512
	vpmovzxbq	(%rcx), %zmm30{%k7}	 # AVX512
	vpmovzxbq	0x123(%rax,%r14,8), %zmm30{%k7}	 # AVX512
	vpmovzxbq	1016(%rdx), %zmm30{%k7}	 # AVX512 Disp8
	vpmovzxbq	1024(%rdx), %zmm30{%k7}	 # AVX512
	vpmovzxbq	-1024(%rdx), %zmm30{%k7}	 # AVX512 Disp8
	vpmovzxbq	-1032(%rdx), %zmm30{%k7}	 # AVX512

	vpmovzxwd	%ymm29, %zmm30{%k7}	 # AVX512
	vpmovzxwd	%ymm29, %zmm30{%k7}{z}	 # AVX512
	vpmovzxwd	(%rcx), %zmm30{%k7}	 # AVX512
	vpmovzxwd	0x123(%rax,%r14,8), %zmm30{%k7}	 # AVX512
	vpmovzxwd	4064(%rdx), %zmm30{%k7}	 # AVX512 Disp8
	vpmovzxwd	4096(%rdx), %zmm30{%k7}	 # AVX512
	vpmovzxwd	-4096(%rdx), %zmm30{%k7}	 # AVX512 Disp8
	vpmovzxwd	-4128(%rdx), %zmm30{%k7}	 # AVX512

	vpmovzxwq	%xmm29, %zmm30{%k7}	 # AVX512
	vpmovzxwq	%xmm29, %zmm30{%k7}{z}	 # AVX512
	vpmovzxwq	(%rcx), %zmm30{%k7}	 # AVX512
	vpmovzxwq	0x123(%rax,%r14,8), %zmm30{%k7}	 # AVX512
	vpmovzxwq	2032(%rdx), %zmm30{%k7}	 # AVX512 Disp8
	vpmovzxwq	2048(%rdx), %zmm30{%k7}	 # AVX512
	vpmovzxwq	-2048(%rdx), %zmm30{%k7}	 # AVX512 Disp8
	vpmovzxwq	-2064(%rdx), %zmm30{%k7}	 # AVX512

	.intel_syntax noprefix
	vextractps	rax, xmm29, 0xab	 # AVX512
	vextractps	rax, xmm29, 123	 # AVX512
	vextractps	r8, xmm29, 123	 # AVX512
	vextractps	DWORD PTR [rcx], xmm29, 123	 # AVX512
	vextractps	DWORD PTR [rax+r14*8+0x1234], xmm29, 123	 # AVX512
	vextractps	DWORD PTR [rdx+508], xmm29, 123	 # AVX512 Disp8
	vextractps	DWORD PTR [rdx+512], xmm29, 123	 # AVX512
	vextractps	DWORD PTR [rdx-512], xmm29, 123	 # AVX512 Disp8
	vextractps	DWORD PTR [rdx-516], xmm29, 123	 # AVX512

	vpmovsxbd	zmm30{k7}, xmm29	 # AVX512
	vpmovsxbd	zmm30{k7}{z}, xmm29	 # AVX512
	vpmovsxbd	zmm30{k7}, XMMWORD PTR [rcx]	 # AVX512
	vpmovsxbd	zmm30{k7}, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vpmovsxbd	zmm30{k7}, XMMWORD PTR [rdx+2032]	 # AVX512 Disp8
	vpmovsxbd	zmm30{k7}, XMMWORD PTR [rdx+2048]	 # AVX512
	vpmovsxbd	zmm30{k7}, XMMWORD PTR [rdx-2048]	 # AVX512 Disp8
	vpmovsxbd	zmm30{k7}, XMMWORD PTR [rdx-2064]	 # AVX512

	vpmovsxbq	zmm30{k7}, xmm29	 # AVX512
	vpmovsxbq	zmm30{k7}{z}, xmm29	 # AVX512
	vpmovsxbq	zmm30{k7}, QWORD PTR [rcx]	 # AVX512
	vpmovsxbq	zmm30{k7}, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vpmovsxbq	zmm30{k7}, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vpmovsxbq	zmm30{k7}, QWORD PTR [rdx+1024]	 # AVX512
	vpmovsxbq	zmm30{k7}, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vpmovsxbq	zmm30{k7}, QWORD PTR [rdx-1032]	 # AVX512

	vpmovsxwd	zmm30{k7}, ymm29	 # AVX512
	vpmovsxwd	zmm30{k7}{z}, ymm29	 # AVX512
	vpmovsxwd	zmm30{k7}, YMMWORD PTR [rcx]	 # AVX512
	vpmovsxwd	zmm30{k7}, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vpmovsxwd	zmm30{k7}, YMMWORD PTR [rdx+4064]	 # AVX512 Disp8
	vpmovsxwd	zmm30{k7}, YMMWORD PTR [rdx+4096]	 # AVX512
	vpmovsxwd	zmm30{k7}, YMMWORD PTR [rdx-4096]	 # AVX512 Disp8
	vpmovsxwd	zmm30{k7}, YMMWORD PTR [rdx-4128]	 # AVX512

	vpmovsxwq	zmm30{k7}, xmm29	 # AVX512
	vpmovsxwq	zmm30{k7}{z}, xmm29	 # AVX512
	vpmovsxwq	zmm30{k7}, XMMWORD PTR [rcx]	 # AVX512
	vpmovsxwq	zmm30{k7}, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vpmovsxwq	zmm30{k7}, XMMWORD PTR [rdx+2032]	 # AVX512 Disp8
	vpmovsxwq	zmm30{k7}, XMMWORD PTR [rdx+2048]	 # AVX512
	vpmovsxwq	zmm30{k7}, XMMWORD PTR [rdx-2048]	 # AVX512 Disp8
	vpmovsxwq	zmm30{k7}, XMMWORD PTR [rdx-2064]	 # AVX512

	vpmovzxbd	zmm30{k7}, xmm29	 # AVX512
	vpmovzxbd	zmm30{k7}{z}, xmm29	 # AVX512
	vpmovzxbd	zmm30{k7}, XMMWORD PTR [rcx]	 # AVX512
	vpmovzxbd	zmm30{k7}, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vpmovzxbd	zmm30{k7}, XMMWORD PTR [rdx+2032]	 # AVX512 Disp8
	vpmovzxbd	zmm30{k7}, XMMWORD PTR [rdx+2048]	 # AVX512
	vpmovzxbd	zmm30{k7}, XMMWORD PTR [rdx-2048]	 # AVX512 Disp8
	vpmovzxbd	zmm30{k7}, XMMWORD PTR [rdx-2064]	 # AVX512

	vpmovzxbq	zmm30{k7}, xmm29	 # AVX512
	vpmovzxbq	zmm30{k7}{z}, xmm29	 # AVX512
	vpmovzxbq	zmm30{k7}, QWORD PTR [rcx]	 # AVX512
	vpmovzxbq	zmm30{k7}, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vpmovzxbq	zmm30{k7}, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vpmovzxbq	zmm30{k7}, QWORD PTR [rdx+1024]	 # AVX512
	vpmovzxbq	zmm30{k7}, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vpmovzxbq	zmm30{k7}, QWORD PTR [rdx-1032]	 # AVX512

	vpmovzxwd	zmm30{k7}, ymm29	 # AVX512
	vpmovzxwd	zmm30{k7}{z}, ymm29	 # AVX512
	vpmovzxwd	zmm30{k7}, YMMWORD PTR [rcx]	 # AVX512
	vpmovzxwd	zmm30{k7}, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vpmovzxwd	zmm30{k7}, YMMWORD PTR [rdx+4064]	 # AVX512 Disp8
	vpmovzxwd	zmm30{k7}, YMMWORD PTR [rdx+4096]	 # AVX512
	vpmovzxwd	zmm30{k7}, YMMWORD PTR [rdx-4096]	 # AVX512 Disp8
	vpmovzxwd	zmm30{k7}, YMMWORD PTR [rdx-4128]	 # AVX512

	vpmovzxwq	zmm30{k7}, xmm29	 # AVX512
	vpmovzxwq	zmm30{k7}{z}, xmm29	 # AVX512
	vpmovzxwq	zmm30{k7}, XMMWORD PTR [rcx]	 # AVX512
	vpmovzxwq	zmm30{k7}, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vpmovzxwq	zmm30{k7}, XMMWORD PTR [rdx+2032]	 # AVX512 Disp8
	vpmovzxwq	zmm30{k7}, XMMWORD PTR [rdx+2048]	 # AVX512
	vpmovzxwq	zmm30{k7}, XMMWORD PTR [rdx-2048]	 # AVX512 Disp8
	vpmovzxwq	zmm30{k7}, XMMWORD PTR [rdx-2064]	 # AVX512

