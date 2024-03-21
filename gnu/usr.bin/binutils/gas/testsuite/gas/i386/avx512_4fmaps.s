# Check 32bit AVX512_4FMAPS instructions

	.allow_index_reg
	.text
_start:
	v4fmaddps	(%ecx), %zmm4, %zmm1	 # AVX512_4FMAPS
	v4fmaddps	(%ecx), %zmm4, %zmm1{%k7}	 # AVX512_4FMAPS
	v4fmaddps	(%ecx), %zmm4, %zmm1{%k7}{z}	 # AVX512_4FMAPS
	v4fmaddps	-123456(%esp,%esi,8), %zmm4, %zmm1	 # AVX512_4FMAPS
	v4fmaddps	0x7f0(%edx), %zmm4, %zmm1	 # AVX512_4FMAPS Disp8
	v4fmaddps	0x800(%edx), %zmm4, %zmm1	 # AVX512_4FMAPS
	v4fmaddps	-0x800(%edx), %zmm4, %zmm1	 # AVX512_4FMAPS Disp8
	v4fmaddps	-0x810(%edx), %zmm4, %zmm1	 # AVX512_4FMAPS
	v4fnmaddps	(%ecx), %zmm4, %zmm1	 # AVX512_4FMAPS
	v4fnmaddps	(%ecx), %zmm4, %zmm1{%k7}	 # AVX512_4FMAPS
	v4fnmaddps	(%ecx), %zmm4, %zmm1{%k7}{z}	 # AVX512_4FMAPS
	v4fnmaddps	-123456(%esp,%esi,8), %zmm4, %zmm1	 # AVX512_4FMAPS
	v4fnmaddps	0x7f0(%edx), %zmm4, %zmm1	 # AVX512_4FMAPS Disp8
	v4fnmaddps	0x800(%edx), %zmm4, %zmm1	 # AVX512_4FMAPS
	v4fnmaddps	-0x800(%edx), %zmm4, %zmm1	 # AVX512_4FMAPS Disp8
	v4fnmaddps	-0x810(%edx), %zmm4, %zmm1	 # AVX512_4FMAPS
	v4fmaddss	(%ecx), %xmm4, %xmm1	 # AVX512_4FMAPS
	v4fmaddss	(%ecx), %xmm4, %xmm1{%k7}	 # AVX512_4FMAPS
	v4fmaddss	(%ecx), %xmm4, %xmm1{%k7}{z}	 # AVX512_4FMAPS
	v4fmaddss	-123456(%esp,%esi,8), %xmm4, %xmm1	 # AVX512_4FMAPS
	v4fmaddss	0x7f0(%edx), %xmm4, %xmm1	 # AVX512_4FMAPS Disp8
	v4fmaddss	0x800(%edx), %xmm4, %xmm1	 # AVX512_4FMAPS
	v4fmaddss	-0x800(%edx), %xmm4, %xmm1	 # AVX512_4FMAPS Disp8
	v4fmaddss	-0x810(%edx), %xmm4, %xmm1	 # AVX512_4FMAPS
	v4fnmaddss	(%ecx), %xmm4, %xmm1	 # AVX512_4FMAPS
	v4fnmaddss	(%ecx), %xmm4, %xmm1{%k7}	 # AVX512_4FMAPS
	v4fnmaddss	(%ecx), %xmm4, %xmm1{%k7}{z}	 # AVX512_4FMAPS
	v4fnmaddss	-123456(%esp,%esi,8), %xmm4, %xmm1	 # AVX512_4FMAPS
	v4fnmaddss	0x7f0(%edx), %xmm4, %xmm1	 # AVX512_4FMAPS Disp8
	v4fnmaddss	0x800(%edx), %xmm4, %xmm1	 # AVX512_4FMAPS
	v4fnmaddss	-0x800(%edx), %xmm4, %xmm1	 # AVX512_4FMAPS Disp8
	v4fnmaddss	-0x810(%edx), %xmm4, %xmm1	 # AVX512_4FMAPS

	.intel_syntax noprefix
	v4fmaddps	zmm1, zmm4, [ecx]	 # AVX512_4FMAPS
	v4fmaddps	zmm1, zmm4, XMMWORD PTR [ecx]	 # AVX512_4FMAPS
	v4fmaddps	zmm1{k7}, zmm4, XMMWORD PTR [ecx]	 # AVX512_4FMAPS
	v4fmaddps	zmm1{k7}{z}, zmm4, XMMWORD PTR [ecx]	 # AVX512_4FMAPS
	v4fmaddps	zmm1, zmm4, XMMWORD PTR [esp+esi*8-123456]	 # AVX512_4FMAPS
	v4fmaddps	zmm1, zmm4, XMMWORD PTR [edx+0x7f0]	 # AVX512_4FMAPS Disp8
	v4fmaddps	zmm1, zmm4, XMMWORD PTR [edx+0x800]	 # AVX512_4FMAPS
	v4fmaddps	zmm1, zmm4, XMMWORD PTR [edx-0x800]	 # AVX512_4FMAPS Disp8
	v4fmaddps	zmm1, zmm4, XMMWORD PTR [edx-0x810]	 # AVX512_4FMAPS
	v4fnmaddps	zmm1, zmm4, [ecx]	 # AVX512_4FMAPS
	v4fnmaddps	zmm1, zmm4, XMMWORD PTR [ecx]	 # AVX512_4FMAPS
	v4fnmaddps	zmm1{k7}, zmm4, XMMWORD PTR [ecx]	 # AVX512_4FMAPS
	v4fnmaddps	zmm1{k7}{z}, zmm4, XMMWORD PTR [ecx]	 # AVX512_4FMAPS
	v4fnmaddps	zmm1, zmm4, XMMWORD PTR [esp+esi*8-123456]	 # AVX512_4FMAPS
	v4fnmaddps	zmm1, zmm4, XMMWORD PTR [edx+0x7f0]	 # AVX512_4FMAPS Disp8
	v4fnmaddps	zmm1, zmm4, XMMWORD PTR [edx+0x800]	 # AVX512_4FMAPS
	v4fnmaddps	zmm1, zmm4, XMMWORD PTR [edx-0x800]	 # AVX512_4FMAPS Disp8
	v4fnmaddps	zmm1, zmm4, XMMWORD PTR [edx-0x810]	 # AVX512_4FMAPS
	v4fmaddss	xmm1, xmm4, [ecx]	 # AVX512_4FMAPS
	v4fmaddss	xmm1, xmm4, XMMWORD PTR [ecx]	 # AVX512_4FMAPS
	v4fmaddss	xmm1{k7}, xmm4, XMMWORD PTR [ecx]	 # AVX512_4FMAPS
	v4fmaddss	xmm1{k7}{z}, xmm4, XMMWORD PTR [ecx]	 # AVX512_4FMAPS
	v4fmaddss	xmm1, xmm4, XMMWORD PTR [esp+esi*8-123456]	 # AVX512_4FMAPS
	v4fmaddss	xmm1, xmm4, XMMWORD PTR [edx+0x7f0]	 # AVX512_4FMAPS Disp8
	v4fmaddss	xmm1, xmm4, XMMWORD PTR [edx+0x800]	 # AVX512_4FMAPS
	v4fmaddss	xmm1, xmm4, XMMWORD PTR [edx-0x800]	 # AVX512_4FMAPS Disp8
	v4fmaddss	xmm1, xmm4, XMMWORD PTR [edx-0x810]	 # AVX512_4FMAPS
	v4fnmaddss	xmm1, xmm4, [ecx]	 # AVX512_4FMAPS
	v4fnmaddss	xmm1, xmm4, XMMWORD PTR [ecx]	 # AVX512_4FMAPS
	v4fnmaddss	xmm1{k7}, xmm4, XMMWORD PTR [ecx]	 # AVX512_4FMAPS
	v4fnmaddss	xmm1{k7}{z}, xmm4, XMMWORD PTR [ecx]	 # AVX512_4FMAPS
	v4fnmaddss	xmm1, xmm4, XMMWORD PTR [esp+esi*8-123456]	 # AVX512_4FMAPS
	v4fnmaddss	xmm1, xmm4, XMMWORD PTR [edx+0x7f0]	 # AVX512_4FMAPS Disp8
	v4fnmaddss	xmm1, xmm4, XMMWORD PTR [edx+0x800]	 # AVX512_4FMAPS
	v4fnmaddss	xmm1, xmm4, XMMWORD PTR [edx-0x800]	 # AVX512_4FMAPS Disp8
	v4fnmaddss	xmm1, xmm4, XMMWORD PTR [edx-0x810]	 # AVX512_4FMAPS
