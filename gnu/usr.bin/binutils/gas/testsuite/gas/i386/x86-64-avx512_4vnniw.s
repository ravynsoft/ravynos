# Check 64bit AVX512_4VNNIW instructions

	.allow_index_reg
	.text
_start:
	vp4dpwssd	(%rcx), %zmm8, %zmm1	 # AVX512_4VNNIW
	vp4dpwssd	(%rcx), %zmm8, %zmm1{%k7}	 # AVX512_4VNNIW
	vp4dpwssd	(%rcx), %zmm8, %zmm1{%k7}{z}	 # AVX512_4VNNIW
	vp4dpwssd	-123456(%rax,%r14,8), %zmm8, %zmm1	 # AVX512_4VNNIW
	vp4dpwssd	4064(%rdx), %zmm8, %zmm1	 # AVX512_4VNNIW Disp8
	vp4dpwssd	4096(%rdx), %zmm8, %zmm1	 # AVX512_4VNNIW
	vp4dpwssd	-4096(%rdx), %zmm8, %zmm1	 # AVX512_4VNNIW Disp8
	vp4dpwssd	-4128(%rdx), %zmm8, %zmm1	 # AVX512_4VNNIW
	vp4dpwssds	(%rcx), %zmm8, %zmm1	 # AVX512_4VNNIW
	vp4dpwssds	(%rcx), %zmm8, %zmm1{%k7}	 # AVX512_4VNNIW
	vp4dpwssds	(%rcx), %zmm8, %zmm1{%k7}{z}	 # AVX512_4VNNIW
	vp4dpwssds	-123456(%rax,%r14,8), %zmm8, %zmm1	 # AVX512_4VNNIW
	vp4dpwssds	4064(%rdx), %zmm8, %zmm1	 # AVX512_4VNNIW Disp8
	vp4dpwssds	4096(%rdx), %zmm8, %zmm1	 # AVX512_4VNNIW
	vp4dpwssds	-4096(%rdx), %zmm8, %zmm1	 # AVX512_4VNNIW Disp8
	vp4dpwssds	-4128(%rdx), %zmm8, %zmm1	 # AVX512_4VNNIW

	.intel_syntax noprefix
	vp4dpwssd	zmm1, zmm8, [rcx]	 # AVX512_4VNNIW
	vp4dpwssd	zmm1, zmm8, XMMWORD PTR [rcx]	 # AVX512_4VNNIW
	vp4dpwssd	zmm1{k7}, zmm8, XMMWORD PTR [rcx]	 # AVX512_4VNNIW
	vp4dpwssd	zmm1{k7}{z}, zmm8, XMMWORD PTR [rcx]	 # AVX512_4VNNIW
	vp4dpwssd	zmm1, zmm8, XMMWORD PTR [rax+r14*8-123456]	 # AVX512_4VNNIW
	vp4dpwssd	zmm1, zmm8, XMMWORD PTR [rdx+4064]	 # AVX512_4VNNIW Disp8
	vp4dpwssd	zmm1, zmm8, XMMWORD PTR [rdx+4096]	 # AVX512_4VNNIW
	vp4dpwssd	zmm1, zmm8, XMMWORD PTR [rdx-4096]	 # AVX512_4VNNIW Disp8
	vp4dpwssd	zmm1, zmm8, XMMWORD PTR [rdx-4128]	 # AVX512_4VNNIW
	vp4dpwssds	zmm1, zmm8, [rcx]	 # AVX512_4VNNIW
	vp4dpwssds	zmm1, zmm8, XMMWORD PTR [rcx]	 # AVX512_4VNNIW
	vp4dpwssds	zmm1{k7}, zmm8, XMMWORD PTR [rcx]	 # AVX512_4VNNIW
	vp4dpwssds	zmm1{k7}{z}, zmm8, XMMWORD PTR [rcx]	 # AVX512_4VNNIW
	vp4dpwssds	zmm1, zmm8, XMMWORD PTR [rax+r14*8-123456]	 # AVX512_4VNNIW
	vp4dpwssds	zmm1, zmm8, XMMWORD PTR [rdx+4064]	 # AVX512_4VNNIW Disp8
	vp4dpwssds	zmm1, zmm8, XMMWORD PTR [rdx+4096]	 # AVX512_4VNNIW
	vp4dpwssds	zmm1, zmm8, XMMWORD PTR [rdx-4096]	 # AVX512_4VNNIW Disp8
	vp4dpwssds	zmm1, zmm8, XMMWORD PTR [rdx-4128]	 # AVX512_4VNNIW
