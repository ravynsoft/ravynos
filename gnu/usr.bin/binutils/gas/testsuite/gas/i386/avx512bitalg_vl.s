# Check 32bit AVX512{BITALG,VL} instructions

	.allow_index_reg
	.text
_start:
	vpshufbitqmb	%xmm4, %xmm5, %k5{%k7}	 # AVX512{BITALG,VL}
	vpshufbitqmb	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512{BITALG,VL}
	vpshufbitqmb	2032(%edx), %xmm5, %k5{%k7}	 # AVX512{BITALG,VL} Disp8
	vpshufbitqmb	%ymm4, %ymm5, %k5{%k7}	 # AVX512{BITALG,VL}
	vpshufbitqmb	-123456(%esp,%esi,8), %ymm5, %k5{%k7}	 # AVX512{BITALG,VL}
	vpshufbitqmb	4064(%edx), %ymm5, %k5{%k7}	 # AVX512{BITALG,VL} Disp8

	vpopcntb	%xmm5, %xmm6{%k7}	 # AVX512{BITALG,VL}
	vpopcntb	%xmm5, %xmm6{%k7}{z}	 # AVX512{BITALG,VL}
	vpopcntb	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{BITALG,VL}
	vpopcntb	2032(%edx), %xmm6{%k7}	 # AVX512{BITALG,VL} Disp8
	vpopcntb	%ymm5, %ymm6{%k7}	 # AVX512{BITALG,VL}
	vpopcntb	%ymm5, %ymm6{%k7}{z}	 # AVX512{BITALG,VL}
	vpopcntb	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{BITALG,VL}
	vpopcntb	4064(%edx), %ymm6{%k7}	 # AVX512{BITALG,VL} Disp8

	vpopcntw	%xmm5, %xmm6{%k7}	 # AVX512{BITALG,VL}
	vpopcntw	%xmm5, %xmm6{%k7}{z}	 # AVX512{BITALG,VL}
	vpopcntw	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{BITALG,VL}
	vpopcntw	2032(%edx), %xmm6{%k7}	 # AVX512{BITALG,VL} Disp8
	vpopcntw	%ymm5, %ymm6{%k7}	 # AVX512{BITALG,VL}
	vpopcntw	%ymm5, %ymm6{%k7}{z}	 # AVX512{BITALG,VL}
	vpopcntw	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{BITALG,VL}
	vpopcntw	4064(%edx), %ymm6{%k7}	 # AVX512{BITALG,VL} Disp8

	vpopcntd	%xmm5, %xmm6{%k7}	 # AVX512{BITALG,VL}
	vpopcntd	%xmm5, %xmm6{%k7}{z}	 # AVX512{BITALG,VL}
	vpopcntd	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{BITALG,VL}
	vpopcntd	2032(%edx), %xmm6{%k7}	 # AVX512{BITALG,VL} Disp8
	vpopcntd	508(%edx){1to4}, %xmm6{%k7}	 # AVX512{BITALG,VL} Disp8
	vpopcntd	%ymm5, %ymm6{%k7}	 # AVX512{BITALG,VL}
	vpopcntd	%ymm5, %ymm6{%k7}{z}	 # AVX512{BITALG,VL}
	vpopcntd	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{BITALG,VL}
	vpopcntd	4064(%edx), %ymm6{%k7}	 # AVX512{BITALG,VL} Disp8
	vpopcntd	508(%edx){1to8}, %ymm6{%k7}	 # AVX512{BITALG,VL} Disp8

	vpopcntq	%xmm5, %xmm6{%k7}	 # AVX512{BITALG,VL}
	vpopcntq	%xmm5, %xmm6{%k7}{z}	 # AVX512{BITALG,VL}
	vpopcntq	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{BITALG,VL}
	vpopcntq	2032(%edx), %xmm6{%k7}	 # AVX512{BITALG,VL} Disp8
	vpopcntq	1016(%edx){1to2}, %xmm6{%k7}	 # AVX512{BITALG,VL} Disp8
	vpopcntq	%ymm5, %ymm6{%k7}	 # AVX512{BITALG,VL}
	vpopcntq	%ymm5, %ymm6{%k7}{z}	 # AVX512{BITALG,VL}
	vpopcntq	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{BITALG,VL}
	vpopcntq	4064(%edx), %ymm6{%k7}	 # AVX512{BITALG,VL} Disp8
	vpopcntq	1016(%edx){1to4}, %ymm6{%k7}	 # AVX512{BITALG,VL} Disp8

	.intel_syntax noprefix
	vpshufbitqmb	k5{k7}, xmm5, xmm4	 # AVX512{BITALG,VL}
	vpshufbitqmb	k5{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BITALG,VL}
	vpshufbitqmb	k5{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BITALG,VL} Disp8
	vpshufbitqmb	k5{k7}, ymm5, ymm4	 # AVX512{BITALG,VL}
	vpshufbitqmb	k5{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BITALG,VL}
	vpshufbitqmb	k5{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BITALG,VL} Disp8

	vpopcntb	xmm6{k7}, xmm5	 # AVX512{BITALG,VL}
	vpopcntb	xmm6{k7}{z}, xmm5	 # AVX512{BITALG,VL}
	vpopcntb	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BITALG,VL}
	vpopcntb	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{BITALG,VL} Disp8
	vpopcntb	ymm6{k7}, ymm5	 # AVX512{BITALG,VL}
	vpopcntb	ymm6{k7}{z}, ymm5	 # AVX512{BITALG,VL}
	vpopcntb	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BITALG,VL}
	vpopcntb	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{BITALG,VL} Disp8

	vpopcntw	xmm6{k7}, xmm5	 # AVX512{BITALG,VL}
	vpopcntw	xmm6{k7}{z}, xmm5	 # AVX512{BITALG,VL}
	vpopcntw	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BITALG,VL}
	vpopcntw	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{BITALG,VL} Disp8
	vpopcntw	ymm6{k7}, ymm5	 # AVX512{BITALG,VL}
	vpopcntw	ymm6{k7}{z}, ymm5	 # AVX512{BITALG,VL}
	vpopcntw	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BITALG,VL}
	vpopcntw	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{BITALG,VL} Disp8

	vpopcntd	xmm6{k7}, xmm5	 # AVX512{BITALG,VL}
	vpopcntd	xmm6{k7}{z}, xmm5	 # AVX512{BITALG,VL}
	vpopcntd	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BITALG,VL}
	vpopcntd	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{BITALG,VL} Disp8
	vpopcntd	xmm6{k7}, [edx+508]{1to4}	 # AVX512{BITALG,VL} Disp8
	vpopcntd	xmm6{k7}, DWORD BCST [edx]	 # AVX512{BITALG,VL}
	vpopcntd	ymm6{k7}, ymm5	 # AVX512{BITALG,VL}
	vpopcntd	ymm6{k7}{z}, ymm5	 # AVX512{BITALG,VL}
	vpopcntd	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BITALG,VL}
	vpopcntd	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{BITALG,VL} Disp8
	vpopcntd	ymm6{k7}, [edx+508]{1to8}	 # AVX512{BITALG,VL} Disp8
	vpopcntd	ymm6{k7}, DWORD BCST [edx]	 # AVX512{BITALG,VL}

	vpopcntq	xmm6{k7}, xmm5	 # AVX512{BITALG,VL}
	vpopcntq	xmm6{k7}{z}, xmm5	 # AVX512{BITALG,VL}
	vpopcntq	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BITALG,VL}
	vpopcntq	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{BITALG,VL} Disp8
	vpopcntq	xmm6{k7}, [edx+1016]{1to2}	 # AVX512{BITALG,VL} Disp8
	vpopcntq	xmm6{k7}, QWORD BCST [edx]	 # AVX512{BITALG,VL}
	vpopcntq	ymm6{k7}, ymm5	 # AVX512{BITALG,VL}
	vpopcntq	ymm6{k7}{z}, ymm5	 # AVX512{BITALG,VL}
	vpopcntq	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BITALG,VL}
	vpopcntq	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{BITALG,VL} Disp8
	vpopcntq	ymm6{k7}, [edx+1016]{1to4}	 # AVX512{BITALG,VL} Disp8
	vpopcntq	ymm6{k7}, QWORD BCST [edx]	 # AVX512{BITALG,VL
