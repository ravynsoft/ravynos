# Check 32bit AVX512{VBMI,VL} instructions

	.allow_index_reg
	.text
_start:
	vpermb	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL}
	vpermb	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{VBMI,VL}
	vpermb	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL}
	vpermb	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL}
	vpermb	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL} Disp8
	vpermb	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL}
	vpermb	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL} Disp8
	vpermb	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL}
	vpermb	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL}
	vpermb	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{VBMI,VL}
	vpermb	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL}
	vpermb	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL}
	vpermb	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL} Disp8
	vpermb	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL}
	vpermb	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL} Disp8
	vpermb	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL}
	vpermi2b	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL}
	vpermi2b	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{VBMI,VL}
	vpermi2b	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL}
	vpermi2b	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL}
	vpermi2b	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL} Disp8
	vpermi2b	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL}
	vpermi2b	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL} Disp8
	vpermi2b	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL}
	vpermi2b	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL}
	vpermi2b	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{VBMI,VL}
	vpermi2b	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL}
	vpermi2b	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL}
	vpermi2b	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL} Disp8
	vpermi2b	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL}
	vpermi2b	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL} Disp8
	vpermi2b	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL}
	vpermt2b	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL}
	vpermt2b	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{VBMI,VL}
	vpermt2b	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL}
	vpermt2b	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL}
	vpermt2b	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL} Disp8
	vpermt2b	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL}
	vpermt2b	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL} Disp8
	vpermt2b	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL}
	vpermt2b	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL}
	vpermt2b	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{VBMI,VL}
	vpermt2b	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL}
	vpermt2b	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL}
	vpermt2b	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL} Disp8
	vpermt2b	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL}
	vpermt2b	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL} Disp8
	vpermt2b	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL}
	vpmultishiftqb	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL}
	vpmultishiftqb	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{VBMI,VL}
	vpmultishiftqb	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL}
	vpmultishiftqb	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL}
	vpmultishiftqb	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL}
	vpmultishiftqb	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL} Disp8
	vpmultishiftqb	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL}
	vpmultishiftqb	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL} Disp8
	vpmultishiftqb	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL}
	vpmultishiftqb	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL} Disp8
	vpmultishiftqb	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL}
	vpmultishiftqb	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL} Disp8
	vpmultishiftqb	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{VBMI,VL}
	vpmultishiftqb	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL}
	vpmultishiftqb	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{VBMI,VL}
	vpmultishiftqb	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL}
	vpmultishiftqb	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL}
	vpmultishiftqb	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL}
	vpmultishiftqb	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL} Disp8
	vpmultishiftqb	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL}
	vpmultishiftqb	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL} Disp8
	vpmultishiftqb	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL}
	vpmultishiftqb	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL} Disp8
	vpmultishiftqb	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL}
	vpmultishiftqb	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL} Disp8
	vpmultishiftqb	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{VBMI,VL}

	.intel_syntax noprefix
	vpermb	xmm6{k7}, xmm5, xmm4	 # AVX512{VBMI,VL}
	vpermb	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{VBMI,VL}
	vpermb	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{VBMI,VL}
	vpermb	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{VBMI,VL}
	vpermb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{VBMI,VL} Disp8
	vpermb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{VBMI,VL}
	vpermb	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{VBMI,VL} Disp8
	vpermb	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{VBMI,VL}
	vpermb	ymm6{k7}, ymm5, ymm4	 # AVX512{VBMI,VL}
	vpermb	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{VBMI,VL}
	vpermb	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{VBMI,VL}
	vpermb	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{VBMI,VL}
	vpermb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{VBMI,VL} Disp8
	vpermb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{VBMI,VL}
	vpermb	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{VBMI,VL} Disp8
	vpermb	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{VBMI,VL}
	vpermi2b	xmm6{k7}, xmm5, xmm4	 # AVX512{VBMI,VL}
	vpermi2b	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{VBMI,VL}
	vpermi2b	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{VBMI,VL}
	vpermi2b	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{VBMI,VL}
	vpermi2b	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{VBMI,VL} Disp8
	vpermi2b	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{VBMI,VL}
	vpermi2b	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{VBMI,VL} Disp8
	vpermi2b	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{VBMI,VL}
	vpermi2b	ymm6{k7}, ymm5, ymm4	 # AVX512{VBMI,VL}
	vpermi2b	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{VBMI,VL}
	vpermi2b	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{VBMI,VL}
	vpermi2b	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{VBMI,VL}
	vpermi2b	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{VBMI,VL} Disp8
	vpermi2b	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{VBMI,VL}
	vpermi2b	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{VBMI,VL} Disp8
	vpermi2b	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{VBMI,VL}
	vpermt2b	xmm6{k7}, xmm5, xmm4	 # AVX512{VBMI,VL}
	vpermt2b	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{VBMI,VL}
	vpermt2b	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{VBMI,VL}
	vpermt2b	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{VBMI,VL}
	vpermt2b	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{VBMI,VL} Disp8
	vpermt2b	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{VBMI,VL}
	vpermt2b	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{VBMI,VL} Disp8
	vpermt2b	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{VBMI,VL}
	vpermt2b	ymm6{k7}, ymm5, ymm4	 # AVX512{VBMI,VL}
	vpermt2b	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{VBMI,VL}
	vpermt2b	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{VBMI,VL}
	vpermt2b	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{VBMI,VL}
	vpermt2b	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{VBMI,VL} Disp8
	vpermt2b	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{VBMI,VL}
	vpermt2b	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{VBMI,VL} Disp8
	vpermt2b	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{VBMI,VL}
	vpmultishiftqb	xmm6{k7}, xmm5, xmm4	 # AVX512{VBMI,VL}
	vpmultishiftqb	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{VBMI,VL}
	vpmultishiftqb	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{VBMI,VL}
	vpmultishiftqb	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{VBMI,VL}
	vpmultishiftqb	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{VBMI,VL}
	vpmultishiftqb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{VBMI,VL} Disp8
	vpmultishiftqb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{VBMI,VL}
	vpmultishiftqb	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{VBMI,VL} Disp8
	vpmultishiftqb	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{VBMI,VL}
	vpmultishiftqb	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{VBMI,VL} Disp8
	vpmultishiftqb	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{VBMI,VL}
	vpmultishiftqb	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{VBMI,VL} Disp8
	vpmultishiftqb	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{VBMI,VL}
	vpmultishiftqb	ymm6{k7}, ymm5, ymm4	 # AVX512{VBMI,VL}
	vpmultishiftqb	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{VBMI,VL}
	vpmultishiftqb	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{VBMI,VL}
	vpmultishiftqb	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{VBMI,VL}
	vpmultishiftqb	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{VBMI,VL}
	vpmultishiftqb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{VBMI,VL} Disp8
	vpmultishiftqb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{VBMI,VL}
	vpmultishiftqb	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{VBMI,VL} Disp8
	vpmultishiftqb	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{VBMI,VL}
	vpmultishiftqb	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{VBMI,VL} Disp8
	vpmultishiftqb	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{VBMI,VL}
	vpmultishiftqb	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{VBMI,VL} Disp8
	vpmultishiftqb	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{VBMI,VL}
