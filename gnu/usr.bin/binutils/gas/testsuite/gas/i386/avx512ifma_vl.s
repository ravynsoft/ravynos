# Check 32bit AVX512{IFMA,VL} instructions

	.allow_index_reg
	.text
_start:
	vpmadd52luq	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{IFMA,VL}
	vpmadd52luq	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{IFMA,VL}
	vpmadd52luq	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{IFMA,VL}
	vpmadd52luq	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{IFMA,VL}
	vpmadd52luq	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{IFMA,VL}
	vpmadd52luq	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{IFMA,VL} Disp8
	vpmadd52luq	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{IFMA,VL}
	vpmadd52luq	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{IFMA,VL} Disp8
	vpmadd52luq	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{IFMA,VL}
	vpmadd52luq	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{IFMA,VL} Disp8
	vpmadd52luq	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{IFMA,VL}
	vpmadd52luq	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{IFMA,VL} Disp8
	vpmadd52luq	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{IFMA,VL}
	vpmadd52luq	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{IFMA,VL}
	vpmadd52luq	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{IFMA,VL}
	vpmadd52luq	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{IFMA,VL}
	vpmadd52luq	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{IFMA,VL}
	vpmadd52luq	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{IFMA,VL}
	vpmadd52luq	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{IFMA,VL} Disp8
	vpmadd52luq	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{IFMA,VL}
	vpmadd52luq	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{IFMA,VL} Disp8
	vpmadd52luq	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{IFMA,VL}
	vpmadd52luq	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{IFMA,VL} Disp8
	vpmadd52luq	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{IFMA,VL}
	vpmadd52luq	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{IFMA,VL} Disp8
	vpmadd52luq	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{IFMA,VL}
	vpmadd52huq	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{IFMA,VL}
	vpmadd52huq	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{IFMA,VL}
	vpmadd52huq	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{IFMA,VL}
	vpmadd52huq	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{IFMA,VL}
	vpmadd52huq	(%eax){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{IFMA,VL}
	vpmadd52huq	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{IFMA,VL} Disp8
	vpmadd52huq	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{IFMA,VL}
	vpmadd52huq	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{IFMA,VL} Disp8
	vpmadd52huq	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{IFMA,VL}
	vpmadd52huq	1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{IFMA,VL} Disp8
	vpmadd52huq	1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{IFMA,VL}
	vpmadd52huq	-1024(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{IFMA,VL} Disp8
	vpmadd52huq	-1032(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512{IFMA,VL}
	vpmadd52huq	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{IFMA,VL}
	vpmadd52huq	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{IFMA,VL}
	vpmadd52huq	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{IFMA,VL}
	vpmadd52huq	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{IFMA,VL}
	vpmadd52huq	(%eax){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{IFMA,VL}
	vpmadd52huq	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{IFMA,VL} Disp8
	vpmadd52huq	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{IFMA,VL}
	vpmadd52huq	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{IFMA,VL} Disp8
	vpmadd52huq	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{IFMA,VL}
	vpmadd52huq	1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{IFMA,VL} Disp8
	vpmadd52huq	1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{IFMA,VL}
	vpmadd52huq	-1024(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{IFMA,VL} Disp8
	vpmadd52huq	-1032(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512{IFMA,VL}

	.intel_syntax noprefix
	vpmadd52luq	xmm6{k7}, xmm5, xmm4	 # AVX512{IFMA,VL}
	vpmadd52luq	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{IFMA,VL}
	vpmadd52luq	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{IFMA,VL}
	vpmadd52luq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{IFMA,VL}
	vpmadd52luq	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{IFMA,VL}
	vpmadd52luq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{IFMA,VL} Disp8
	vpmadd52luq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{IFMA,VL}
	vpmadd52luq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{IFMA,VL} Disp8
	vpmadd52luq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{IFMA,VL}
	vpmadd52luq	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{IFMA,VL} Disp8
	vpmadd52luq	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{IFMA,VL}
	vpmadd52luq	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{IFMA,VL} Disp8
	vpmadd52luq	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{IFMA,VL}
	vpmadd52luq	ymm6{k7}, ymm5, ymm4	 # AVX512{IFMA,VL}
	vpmadd52luq	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{IFMA,VL}
	vpmadd52luq	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{IFMA,VL}
	vpmadd52luq	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{IFMA,VL}
	vpmadd52luq	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{IFMA,VL}
	vpmadd52luq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{IFMA,VL} Disp8
	vpmadd52luq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{IFMA,VL}
	vpmadd52luq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{IFMA,VL} Disp8
	vpmadd52luq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{IFMA,VL}
	vpmadd52luq	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{IFMA,VL} Disp8
	vpmadd52luq	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{IFMA,VL}
	vpmadd52luq	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{IFMA,VL} Disp8
	vpmadd52luq	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{IFMA,VL}
	vpmadd52huq	xmm6{k7}, xmm5, xmm4	 # AVX512{IFMA,VL}
	vpmadd52huq	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{IFMA,VL}
	vpmadd52huq	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{IFMA,VL}
	vpmadd52huq	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{IFMA,VL}
	vpmadd52huq	xmm6{k7}, xmm5, [eax]{1to2}	 # AVX512{IFMA,VL}
	vpmadd52huq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{IFMA,VL} Disp8
	vpmadd52huq	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{IFMA,VL}
	vpmadd52huq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{IFMA,VL} Disp8
	vpmadd52huq	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{IFMA,VL}
	vpmadd52huq	xmm6{k7}, xmm5, [edx+1016]{1to2}	 # AVX512{IFMA,VL} Disp8
	vpmadd52huq	xmm6{k7}, xmm5, [edx+1024]{1to2}	 # AVX512{IFMA,VL}
	vpmadd52huq	xmm6{k7}, xmm5, [edx-1024]{1to2}	 # AVX512{IFMA,VL} Disp8
	vpmadd52huq	xmm6{k7}, xmm5, [edx-1032]{1to2}	 # AVX512{IFMA,VL}
	vpmadd52huq	ymm6{k7}, ymm5, ymm4	 # AVX512{IFMA,VL}
	vpmadd52huq	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{IFMA,VL}
	vpmadd52huq	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{IFMA,VL}
	vpmadd52huq	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{IFMA,VL}
	vpmadd52huq	ymm6{k7}, ymm5, [eax]{1to4}	 # AVX512{IFMA,VL}
	vpmadd52huq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{IFMA,VL} Disp8
	vpmadd52huq	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{IFMA,VL}
	vpmadd52huq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{IFMA,VL} Disp8
	vpmadd52huq	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{IFMA,VL}
	vpmadd52huq	ymm6{k7}, ymm5, [edx+1016]{1to4}	 # AVX512{IFMA,VL} Disp8
	vpmadd52huq	ymm6{k7}, ymm5, [edx+1024]{1to4}	 # AVX512{IFMA,VL}
	vpmadd52huq	ymm6{k7}, ymm5, [edx-1024]{1to4}	 # AVX512{IFMA,VL} Disp8
	vpmadd52huq	ymm6{k7}, ymm5, [edx-1032]{1to4}	 # AVX512{IFMA,VL}
