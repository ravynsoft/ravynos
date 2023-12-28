# Check 32bit AVX512_4VNNIW instructions

	.allow_index_reg
	.text
_start:
	vp4dpwssd	(%ecx), %zmm4, %zmm1	 # AVX512_4VNNIW
	vp4dpwssd	(%ecx), %zmm4, %zmm1{%k7}	 # AVX512_4VNNIW
	vp4dpwssd	(%ecx), %zmm4, %zmm1{%k7}{z}	 # AVX512_4VNNIW
	vp4dpwssd	-123456(%esp,%esi,8), %zmm4, %zmm1	 # AVX512_4VNNIW
	vp4dpwssd	4064(%edx), %zmm4, %zmm1	 # AVX512_4VNNIW Disp8
	vp4dpwssd	4096(%edx), %zmm4, %zmm1	 # AVX512_4VNNIW
	vp4dpwssd	-4096(%edx), %zmm4, %zmm1	 # AVX512_4VNNIW Disp8
	vp4dpwssd	-4128(%edx), %zmm4, %zmm1	 # AVX512_4VNNIW
	vp4dpwssds	(%ecx), %zmm4, %zmm1	 # AVX512_4VNNIW
	vp4dpwssds	(%ecx), %zmm4, %zmm1{%k7}	 # AVX512_4VNNIW
	vp4dpwssds	(%ecx), %zmm4, %zmm1{%k7}{z}	 # AVX512_4VNNIW
	vp4dpwssds	-123456(%esp,%esi,8), %zmm4, %zmm1	 # AVX512_4VNNIW
	vp4dpwssds	4064(%edx), %zmm4, %zmm1	 # AVX512_4VNNIW Disp8
	vp4dpwssds	4096(%edx), %zmm4, %zmm1	 # AVX512_4VNNIW
	vp4dpwssds	-4096(%edx), %zmm4, %zmm1	 # AVX512_4VNNIW Disp8
	vp4dpwssds	-4128(%edx), %zmm4, %zmm1	 # AVX512_4VNNIW

	.intel_syntax noprefix
	vp4dpwssd	zmm1, zmm4, [ecx]	 # AVX512_4VNNIW
	vp4dpwssd	zmm1, zmm4, XMMWORD PTR [ecx]	 # AVX512_4VNNIW
	vp4dpwssd	zmm1{k7}, zmm4, XMMWORD PTR [ecx]	 # AVX512_4VNNIW
	vp4dpwssd	zmm1{k7}{z}, zmm4, XMMWORD PTR [ecx]	 # AVX512_4VNNIW
	vp4dpwssd	zmm1, zmm4, XMMWORD PTR [esp+esi*8-123456]	 # AVX512_4VNNIW
	vp4dpwssd	zmm1, zmm4, XMMWORD PTR [edx+4064]	 # AVX512_4VNNIW Disp8
	vp4dpwssd	zmm1, zmm4, XMMWORD PTR [edx+4096]	 # AVX512_4VNNIW
	vp4dpwssd	zmm1, zmm4, XMMWORD PTR [edx-4096]	 # AVX512_4VNNIW Disp8
	vp4dpwssd	zmm1, zmm4, XMMWORD PTR [edx-4128]	 # AVX512_4VNNIW
	vp4dpwssds	zmm1, zmm4, [ecx]	 # AVX512_4VNNIW
	vp4dpwssds	zmm1, zmm4, XMMWORD PTR [ecx]	 # AVX512_4VNNIW
	vp4dpwssds	zmm1{k7}, zmm4, XMMWORD PTR [ecx]	 # AVX512_4VNNIW
	vp4dpwssds	zmm1{k7}{z}, zmm4, XMMWORD PTR [ecx]	 # AVX512_4VNNIW
	vp4dpwssds	zmm1, zmm4, XMMWORD PTR [esp+esi*8-123456]	 # AVX512_4VNNIW
	vp4dpwssds	zmm1, zmm4, XMMWORD PTR [edx+4064]	 # AVX512_4VNNIW Disp8
	vp4dpwssds	zmm1, zmm4, XMMWORD PTR [edx+4096]	 # AVX512_4VNNIW
	vp4dpwssds	zmm1, zmm4, XMMWORD PTR [edx-4096]	 # AVX512_4VNNIW Disp8
	vp4dpwssds	zmm1, zmm4, XMMWORD PTR [edx-4128]	 # AVX512_4VNNIW
