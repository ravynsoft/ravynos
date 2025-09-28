# Check EVEX WIG instructions

	.allow_index_reg
	.text
_start:

	{evex} vcvtsi2ss %eax, %xmm0, %xmm0
	{evex} vcvtsi2ss 4(%eax), %xmm0, %xmm0

	{evex} vcvtsi2sd %eax, %xmm0, %xmm0
	{evex} vcvtsi2sd 4(%eax), %xmm0, %xmm0

	{evex} vcvtss2si %xmm0, %eax

	{evex} vcvtsd2si %xmm0, %eax

	{evex} vcvttss2si %xmm0, %eax

	{evex} vcvttsd2si %xmm0, %eax

	vcvtusi2ss %eax, %xmm0, %xmm0
	vcvtusi2ss 4(%eax), %xmm0, %xmm0

	vcvtusi2sd %eax, %xmm0, %xmm0
	vcvtusi2sd 4(%eax), %xmm0, %xmm0

	vcvtss2usi %xmm0, %eax

	vcvtsd2usi %xmm0, %eax

	vcvttss2usi %xmm0, %eax

	vcvttsd2usi %xmm0, %eax

	{evex} vextractps $0, %xmm0, %eax
	{evex} vextractps $0, %xmm0, 4(%eax)

	{evex} vmovd %eax, %xmm0
	{evex} vmovd 4(%eax), %xmm0

	{evex} vmovd %xmm0, %eax
	{evex} vmovd %xmm0, 4(%eax)

	vpbroadcastd %eax, %xmm0

	{evex} vpextrb $0, %xmm0, %eax
	{evex} vpextrb $0, %xmm0, 1(%eax)

	{evex} vpextrd $0, %xmm0, %eax
	{evex} vpextrd $0, %xmm0, 4(%eax)

	{evex} vpextrw $0, %xmm0, %eax
	{evex} {store} vpextrw $0, %xmm0, %eax
	{evex} vpextrw $0, %xmm0, 2(%eax)

	{evex} vpinsrb $0, %eax, %xmm0, %xmm0
	{evex} vpinsrb $0, 1(%eax), %xmm0, %xmm0

	{evex} vpinsrd $0, %eax, %xmm0, %xmm0
	{evex} vpinsrd $0, 4(%eax), %xmm0, %xmm0

	{evex} vpinsrw $0, %eax, %xmm0, %xmm0
	{evex} vpinsrw $0, 2(%eax), %xmm0, %xmm0

	vmovss %xmm0, %xmm0, %xmm0{%k7}
	vmovss (%eax), %xmm0{%k7}
	vmovss %xmm0, (%eax){%k7}

	vmovsd %xmm0, %xmm0, %xmm0{%k7}
	vmovsd (%eax), %xmm0{%k7}
	vmovsd %xmm0, (%eax){%k7}

	vmovsh %xmm0, %xmm0, %xmm0{%k7}
	vmovsh (%eax), %xmm0{%k7}
	vmovsh %xmm0, (%eax){%k7}

	vpmovsxbd	%xmm5, %zmm6{%k7}	 # AVX512
	vpmovsxbd	%xmm5, %zmm6{%k7}{z}	 # AVX512
	vpmovsxbd	(%ecx), %zmm6{%k7}	 # AVX512
	vpmovsxbd	-123456(%esp,%esi,8), %zmm6{%k7}	 # AVX512
	vpmovsxbd	2032(%edx), %zmm6{%k7}	 # AVX512 Disp8
	vpmovsxbd	2048(%edx), %zmm6{%k7}	 # AVX512
	vpmovsxbd	-2048(%edx), %zmm6{%k7}	 # AVX512 Disp8
	vpmovsxbd	-2064(%edx), %zmm6{%k7}	 # AVX512

	vpmovsxbq	%xmm5, %zmm6{%k7}	 # AVX512
	vpmovsxbq	%xmm5, %zmm6{%k7}{z}	 # AVX512
	vpmovsxbq	(%ecx), %zmm6{%k7}	 # AVX512
	vpmovsxbq	-123456(%esp,%esi,8), %zmm6{%k7}	 # AVX512
	vpmovsxbq	1016(%edx), %zmm6{%k7}	 # AVX512 Disp8
	vpmovsxbq	1024(%edx), %zmm6{%k7}	 # AVX512
	vpmovsxbq	-1024(%edx), %zmm6{%k7}	 # AVX512 Disp8
	vpmovsxbq	-1032(%edx), %zmm6{%k7}	 # AVX512

	vpmovsxwd	%ymm5, %zmm6{%k7}	 # AVX512
	vpmovsxwd	%ymm5, %zmm6{%k7}{z}	 # AVX512
	vpmovsxwd	(%ecx), %zmm6{%k7}	 # AVX512
	vpmovsxwd	-123456(%esp,%esi,8), %zmm6{%k7}	 # AVX512
	vpmovsxwd	4064(%edx), %zmm6{%k7}	 # AVX512 Disp8
	vpmovsxwd	4096(%edx), %zmm6{%k7}	 # AVX512
	vpmovsxwd	-4096(%edx), %zmm6{%k7}	 # AVX512 Disp8
	vpmovsxwd	-4128(%edx), %zmm6{%k7}	 # AVX512

	vpmovsxwq	%xmm5, %zmm6{%k7}	 # AVX512
	vpmovsxwq	%xmm5, %zmm6{%k7}{z}	 # AVX512
	vpmovsxwq	(%ecx), %zmm6{%k7}	 # AVX512
	vpmovsxwq	-123456(%esp,%esi,8), %zmm6{%k7}	 # AVX512
	vpmovsxwq	2032(%edx), %zmm6{%k7}	 # AVX512 Disp8
	vpmovsxwq	2048(%edx), %zmm6{%k7}	 # AVX512
	vpmovsxwq	-2048(%edx), %zmm6{%k7}	 # AVX512 Disp8
	vpmovsxwq	-2064(%edx), %zmm6{%k7}	 # AVX512

	vpmovzxbd	%xmm5, %zmm6{%k7}	 # AVX512
	vpmovzxbd	%xmm5, %zmm6{%k7}{z}	 # AVX512
	vpmovzxbd	(%ecx), %zmm6{%k7}	 # AVX512
	vpmovzxbd	-123456(%esp,%esi,8), %zmm6{%k7}	 # AVX512
	vpmovzxbd	2032(%edx), %zmm6{%k7}	 # AVX512 Disp8
	vpmovzxbd	2048(%edx), %zmm6{%k7}	 # AVX512
	vpmovzxbd	-2048(%edx), %zmm6{%k7}	 # AVX512 Disp8
	vpmovzxbd	-2064(%edx), %zmm6{%k7}	 # AVX512

	vpmovzxbq	%xmm5, %zmm6{%k7}	 # AVX512
	vpmovzxbq	%xmm5, %zmm6{%k7}{z}	 # AVX512
	vpmovzxbq	(%ecx), %zmm6{%k7}	 # AVX512
	vpmovzxbq	-123456(%esp,%esi,8), %zmm6{%k7}	 # AVX512
	vpmovzxbq	1016(%edx), %zmm6{%k7}	 # AVX512 Disp8
	vpmovzxbq	1024(%edx), %zmm6{%k7}	 # AVX512
	vpmovzxbq	-1024(%edx), %zmm6{%k7}	 # AVX512 Disp8
	vpmovzxbq	-1032(%edx), %zmm6{%k7}	 # AVX512

	vpmovzxwd	%ymm5, %zmm6{%k7}	 # AVX512
	vpmovzxwd	%ymm5, %zmm6{%k7}{z}	 # AVX512
	vpmovzxwd	(%ecx), %zmm6{%k7}	 # AVX512
	vpmovzxwd	-123456(%esp,%esi,8), %zmm6{%k7}	 # AVX512
	vpmovzxwd	4064(%edx), %zmm6{%k7}	 # AVX512 Disp8
	vpmovzxwd	4096(%edx), %zmm6{%k7}	 # AVX512
	vpmovzxwd	-4096(%edx), %zmm6{%k7}	 # AVX512 Disp8
	vpmovzxwd	-4128(%edx), %zmm6{%k7}	 # AVX512

	vpmovzxwq	%xmm5, %zmm6{%k7}	 # AVX512
	vpmovzxwq	%xmm5, %zmm6{%k7}{z}	 # AVX512
	vpmovzxwq	(%ecx), %zmm6{%k7}	 # AVX512
	vpmovzxwq	-123456(%esp,%esi,8), %zmm6{%k7}	 # AVX512
	vpmovzxwq	2032(%edx), %zmm6{%k7}	 # AVX512 Disp8
	vpmovzxwq	2048(%edx), %zmm6{%k7}	 # AVX512
	vpmovzxwq	-2048(%edx), %zmm6{%k7}	 # AVX512 Disp8
	vpmovzxwq	-2064(%edx), %zmm6{%k7}	 # AVX512

	.intel_syntax noprefix
	vpmovsxbd	zmm6{k7}, xmm5	 # AVX512
	vpmovsxbd	zmm6{k7}{z}, xmm5	 # AVX512
	vpmovsxbd	zmm6{k7}, XMMWORD PTR [ecx]	 # AVX512
	vpmovsxbd	zmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512
	vpmovsxbd	zmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512 Disp8
	vpmovsxbd	zmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512
	vpmovsxbd	zmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512 Disp8
	vpmovsxbd	zmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512

	vpmovsxbq	zmm6{k7}, xmm5	 # AVX512
	vpmovsxbq	zmm6{k7}{z}, xmm5	 # AVX512
	vpmovsxbq	zmm6{k7}, QWORD PTR [ecx]	 # AVX512
	vpmovsxbq	zmm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vpmovsxbq	zmm6{k7}, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vpmovsxbq	zmm6{k7}, QWORD PTR [edx+1024]	 # AVX512
	vpmovsxbq	zmm6{k7}, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vpmovsxbq	zmm6{k7}, QWORD PTR [edx-1032]	 # AVX512

	vpmovsxwd	zmm6{k7}, ymm5	 # AVX512
	vpmovsxwd	zmm6{k7}{z}, ymm5	 # AVX512
	vpmovsxwd	zmm6{k7}, YMMWORD PTR [ecx]	 # AVX512
	vpmovsxwd	zmm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512
	vpmovsxwd	zmm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512 Disp8
	vpmovsxwd	zmm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512
	vpmovsxwd	zmm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512 Disp8
	vpmovsxwd	zmm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512

	vpmovsxwq	zmm6{k7}, xmm5	 # AVX512
	vpmovsxwq	zmm6{k7}{z}, xmm5	 # AVX512
	vpmovsxwq	zmm6{k7}, XMMWORD PTR [ecx]	 # AVX512
	vpmovsxwq	zmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512
	vpmovsxwq	zmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512 Disp8
	vpmovsxwq	zmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512
	vpmovsxwq	zmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512 Disp8
	vpmovsxwq	zmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512

	vpmovzxbd	zmm6{k7}, xmm5	 # AVX512
	vpmovzxbd	zmm6{k7}{z}, xmm5	 # AVX512
	vpmovzxbd	zmm6{k7}, XMMWORD PTR [ecx]	 # AVX512
	vpmovzxbd	zmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512
	vpmovzxbd	zmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512 Disp8
	vpmovzxbd	zmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512
	vpmovzxbd	zmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512 Disp8
	vpmovzxbd	zmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512

	vpmovzxbq	zmm6{k7}, xmm5	 # AVX512
	vpmovzxbq	zmm6{k7}{z}, xmm5	 # AVX512
	vpmovzxbq	zmm6{k7}, QWORD PTR [ecx]	 # AVX512
	vpmovzxbq	zmm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vpmovzxbq	zmm6{k7}, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vpmovzxbq	zmm6{k7}, QWORD PTR [edx+1024]	 # AVX512
	vpmovzxbq	zmm6{k7}, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vpmovzxbq	zmm6{k7}, QWORD PTR [edx-1032]	 # AVX512

	vpmovzxwd	zmm6{k7}, ymm5	 # AVX512
	vpmovzxwd	zmm6{k7}{z}, ymm5	 # AVX512
	vpmovzxwd	zmm6{k7}, YMMWORD PTR [ecx]	 # AVX512
	vpmovzxwd	zmm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512
	vpmovzxwd	zmm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512 Disp8
	vpmovzxwd	zmm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512
	vpmovzxwd	zmm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512 Disp8
	vpmovzxwd	zmm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512

	vpmovzxwq	zmm6{k7}, xmm5	 # AVX512
	vpmovzxwq	zmm6{k7}{z}, xmm5	 # AVX512
	vpmovzxwq	zmm6{k7}, XMMWORD PTR [ecx]	 # AVX512
	vpmovzxwq	zmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512
	vpmovzxwq	zmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512 Disp8
	vpmovzxwq	zmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512
	vpmovzxwq	zmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512 Disp8
	vpmovzxwq	zmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512

