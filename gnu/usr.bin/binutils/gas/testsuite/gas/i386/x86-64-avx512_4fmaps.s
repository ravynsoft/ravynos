# Check 64bit AVX512_4FMAPS instructions

	.allow_index_reg
	.text
_start:
	v4fmaddps	(%rcx), %zmm8, %zmm1	 # AVX512_4FMAPS
	v4fmaddps	(%rcx), %zmm8, %zmm1{%k7}	 # AVX512_4FMAPS
	v4fmaddps	(%rcx), %zmm8, %zmm1{%k7}{z}	 # AVX512_4FMAPS
	v4fmaddps	-123456(%rax,%r14,8), %zmm8, %zmm1	 # AVX512_4FMAPS
	v4fmaddps	0x7f0(%rdx), %zmm8, %zmm1	 # AVX512_4FMAPS Disp8
	v4fmaddps	0x800(%rdx), %zmm8, %zmm1	 # AVX512_4FMAPS
	v4fmaddps	-0x800(%rdx), %zmm8, %zmm1	 # AVX512_4FMAPS Disp8
	v4fmaddps	-0x810(%rdx), %zmm8, %zmm1	 # AVX512_4FMAPS
	v4fnmaddps	(%rcx), %zmm8, %zmm1	 # AVX512_4FMAPS
	v4fnmaddps	(%rcx), %zmm8, %zmm1{%k7}	 # AVX512_4FMAPS
	v4fnmaddps	(%rcx), %zmm8, %zmm1{%k7}{z}	 # AVX512_4FMAPS
	v4fnmaddps	-123456(%rax,%r14,8), %zmm8, %zmm1	 # AVX512_4FMAPS
	v4fnmaddps	0x7f0(%rdx), %zmm8, %zmm1	 # AVX512_4FMAPS Disp8
	v4fnmaddps	0x800(%rdx), %zmm8, %zmm1	 # AVX512_4FMAPS
	v4fnmaddps	-0x800(%rdx), %zmm8, %zmm1	 # AVX512_4FMAPS Disp8
	v4fnmaddps	-0x810(%rdx), %zmm8, %zmm1	 # AVX512_4FMAPS
	v4fmaddss	(%rcx), %xmm8, %xmm1	 # AVX512_4FMAPS
	v4fmaddss	(%rcx), %xmm8, %xmm1{%k7}	 # AVX512_4FMAPS
	v4fmaddss	(%rcx), %xmm8, %xmm1{%k7}{z}	 # AVX512_4FMAPS
	v4fmaddss	-123456(%rax,%r14,8), %xmm8, %xmm1	 # AVX512_4FMAPS
	v4fmaddss	0x7f0(%rdx), %xmm8, %xmm1	 # AVX512_4FMAPS Disp8
	v4fmaddss	0x800(%rdx), %xmm8, %xmm1	 # AVX512_4FMAPS
	v4fmaddss	-0x800(%rdx), %xmm8, %xmm1	 # AVX512_4FMAPS Disp8
	v4fmaddss	-0x810(%rdx), %xmm8, %xmm1	 # AVX512_4FMAPS
	v4fnmaddss	(%rcx), %xmm8, %xmm1	 # AVX512_4FMAPS
	v4fnmaddss	(%rcx), %xmm8, %xmm1{%k7}	 # AVX512_4FMAPS
	v4fnmaddss	(%rcx), %xmm8, %xmm1{%k7}{z}	 # AVX512_4FMAPS
	v4fnmaddss	-123456(%rax,%r14,8), %xmm8, %xmm1	 # AVX512_4FMAPS
	v4fnmaddss	0x7f0(%rdx), %xmm8, %xmm1	 # AVX512_4FMAPS Disp8
	v4fnmaddss	0x800(%rdx), %xmm8, %xmm1	 # AVX512_4FMAPS
	v4fnmaddss	-0x800(%rdx), %xmm8, %xmm1	 # AVX512_4FMAPS Disp8
	v4fnmaddss	-0x810(%rdx), %xmm8, %xmm1	 # AVX512_4FMAPS

	.intel_syntax noprefix
	v4fmaddps	zmm1, zmm8, [rcx]	 # AVX512_4FMAPS
	v4fmaddps	zmm1, zmm8, XMMWORD PTR [rcx]	 # AVX512_4FMAPS
	v4fmaddps	zmm1{k7}, zmm8, XMMWORD PTR [rcx]	 # AVX512_4FMAPS
	v4fmaddps	zmm1{k7}{z}, zmm8, XMMWORD PTR [rcx]	 # AVX512_4FMAPS
	v4fmaddps	zmm1, zmm8, XMMWORD PTR [rax+r14*8-123456]	 # AVX512_4FMAPS
	v4fmaddps	zmm1, zmm8, XMMWORD PTR [rdx+0x7f0]	 # AVX512_4FMAPS Disp8
	v4fmaddps	zmm1, zmm8, XMMWORD PTR [rdx+0x800]	 # AVX512_4FMAPS
	v4fmaddps	zmm1, zmm8, XMMWORD PTR [rdx-0x800]	 # AVX512_4FMAPS Disp8
	v4fmaddps	zmm1, zmm8, XMMWORD PTR [rdx-0x810]	 # AVX512_4FMAPS
	v4fnmaddps	zmm1, zmm8, [rcx]	 # AVX512_4FMAPS
	v4fnmaddps	zmm1, zmm8, XMMWORD PTR [rcx]	 # AVX512_4FMAPS
	v4fnmaddps	zmm1{k7}, zmm8, XMMWORD PTR [rcx]	 # AVX512_4FMAPS
	v4fnmaddps	zmm1{k7}{z}, zmm8, XMMWORD PTR [rcx]	 # AVX512_4FMAPS
	v4fnmaddps	zmm1, zmm8, XMMWORD PTR [rax+r14*8-123456]	 # AVX512_4FMAPS
	v4fnmaddps	zmm1, zmm8, XMMWORD PTR [rdx+0x7f0]	 # AVX512_4FMAPS Disp8
	v4fnmaddps	zmm1, zmm8, XMMWORD PTR [rdx+0x800]	 # AVX512_4FMAPS
	v4fnmaddps	zmm1, zmm8, XMMWORD PTR [rdx-0x800]	 # AVX512_4FMAPS Disp8
	v4fnmaddps	zmm1, zmm8, XMMWORD PTR [rdx-0x810]	 # AVX512_4FMAPS
	v4fmaddss	xmm1, xmm8, [rcx]	 # AVX512_4FMAPS
	v4fmaddss	xmm1, xmm8, XMMWORD PTR [rcx]	 # AVX512_4FMAPS
	v4fmaddss	xmm1{k7}, xmm8, XMMWORD PTR [rcx]	 # AVX512_4FMAPS
	v4fmaddss	xmm1{k7}{z}, xmm8, XMMWORD PTR [rcx]	 # AVX512_4FMAPS
	v4fmaddss	xmm1, xmm8, XMMWORD PTR [rax+r14*8-123456]	 # AVX512_4FMAPS
	v4fmaddss	xmm1, xmm8, XMMWORD PTR [rdx+0x7f0]	 # AVX512_4FMAPS Disp8
	v4fmaddss	xmm1, xmm8, XMMWORD PTR [rdx+0x800]	 # AVX512_4FMAPS
	v4fmaddss	xmm1, xmm8, XMMWORD PTR [rdx-0x800]	 # AVX512_4FMAPS Disp8
	v4fmaddss	xmm1, xmm8, XMMWORD PTR [rdx-0x810]	 # AVX512_4FMAPS
	v4fnmaddss	xmm1, xmm8, [rcx]	 # AVX512_4FMAPS
	v4fnmaddss	xmm1, xmm8, XMMWORD PTR [rcx]	 # AVX512_4FMAPS
	v4fnmaddss	xmm1{k7}, xmm8, XMMWORD PTR [rcx]	 # AVX512_4FMAPS
	v4fnmaddss	xmm1{k7}{z}, xmm8, XMMWORD PTR [rcx]	 # AVX512_4FMAPS
	v4fnmaddss	xmm1, xmm8, XMMWORD PTR [rax+r14*8-123456]	 # AVX512_4FMAPS
	v4fnmaddss	xmm1, xmm8, XMMWORD PTR [rdx+0x7f0]	 # AVX512_4FMAPS Disp8
	v4fnmaddss	xmm1, xmm8, XMMWORD PTR [rdx+0x800]	 # AVX512_4FMAPS
	v4fnmaddss	xmm1, xmm8, XMMWORD PTR [rdx-0x800]	 # AVX512_4FMAPS Disp8
	v4fnmaddss	xmm1, xmm8, XMMWORD PTR [rdx-0x810]	 # AVX512_4FMAPS
