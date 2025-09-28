# Check 32bit AVX512{CD,VL} instructions

	.allow_index_reg
	.text
_start:
	vpconflictd	%xmm5, %xmm6{%k7}	 # AVX512{CD,VL}
	vpconflictd	%xmm5, %xmm6{%k7}{z}	 # AVX512{CD,VL}
	vpconflictd	(%ecx), %xmm6{%k7}	 # AVX512{CD,VL}
	vpconflictd	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{CD,VL}
	vpconflictd	(%eax){1to4}, %xmm6{%k7}	 # AVX512{CD,VL}
	vpconflictd	2032(%edx), %xmm6{%k7}	 # AVX512{CD,VL} Disp8
	vpconflictd	2048(%edx), %xmm6{%k7}	 # AVX512{CD,VL}
	vpconflictd	-2048(%edx), %xmm6{%k7}	 # AVX512{CD,VL} Disp8
	vpconflictd	-2064(%edx), %xmm6{%k7}	 # AVX512{CD,VL}
	vpconflictd	508(%edx){1to4}, %xmm6{%k7}	 # AVX512{CD,VL} Disp8
	vpconflictd	512(%edx){1to4}, %xmm6{%k7}	 # AVX512{CD,VL}
	vpconflictd	-512(%edx){1to4}, %xmm6{%k7}	 # AVX512{CD,VL} Disp8
	vpconflictd	-516(%edx){1to4}, %xmm6{%k7}	 # AVX512{CD,VL}
	vpconflictd	%ymm5, %ymm6{%k7}	 # AVX512{CD,VL}
	vpconflictd	%ymm5, %ymm6{%k7}{z}	 # AVX512{CD,VL}
	vpconflictd	(%ecx), %ymm6{%k7}	 # AVX512{CD,VL}
	vpconflictd	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{CD,VL}
	vpconflictd	(%eax){1to8}, %ymm6{%k7}	 # AVX512{CD,VL}
	vpconflictd	4064(%edx), %ymm6{%k7}	 # AVX512{CD,VL} Disp8
	vpconflictd	4096(%edx), %ymm6{%k7}	 # AVX512{CD,VL}
	vpconflictd	-4096(%edx), %ymm6{%k7}	 # AVX512{CD,VL} Disp8
	vpconflictd	-4128(%edx), %ymm6{%k7}	 # AVX512{CD,VL}
	vpconflictd	508(%edx){1to8}, %ymm6{%k7}	 # AVX512{CD,VL} Disp8
	vpconflictd	512(%edx){1to8}, %ymm6{%k7}	 # AVX512{CD,VL}
	vpconflictd	-512(%edx){1to8}, %ymm6{%k7}	 # AVX512{CD,VL} Disp8
	vpconflictd	-516(%edx){1to8}, %ymm6{%k7}	 # AVX512{CD,VL}
	vpconflictq	%xmm5, %xmm6{%k7}	 # AVX512{CD,VL}
	vpconflictq	%xmm5, %xmm6{%k7}{z}	 # AVX512{CD,VL}
	vpconflictq	(%ecx), %xmm6{%k7}	 # AVX512{CD,VL}
	vpconflictq	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{CD,VL}
	vpconflictq	(%eax){1to2}, %xmm6{%k7}	 # AVX512{CD,VL}
	vpconflictq	2032(%edx), %xmm6{%k7}	 # AVX512{CD,VL} Disp8
	vpconflictq	2048(%edx), %xmm6{%k7}	 # AVX512{CD,VL}
	vpconflictq	-2048(%edx), %xmm6{%k7}	 # AVX512{CD,VL} Disp8
	vpconflictq	-2064(%edx), %xmm6{%k7}	 # AVX512{CD,VL}
	vpconflictq	1016(%edx){1to2}, %xmm6{%k7}	 # AVX512{CD,VL} Disp8
	vpconflictq	1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{CD,VL}
	vpconflictq	-1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{CD,VL} Disp8
	vpconflictq	-1032(%edx){1to2}, %xmm6{%k7}	 # AVX512{CD,VL}
	vpconflictq	%ymm5, %ymm6{%k7}	 # AVX512{CD,VL}
	vpconflictq	%ymm5, %ymm6{%k7}{z}	 # AVX512{CD,VL}
	vpconflictq	(%ecx), %ymm6{%k7}	 # AVX512{CD,VL}
	vpconflictq	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{CD,VL}
	vpconflictq	(%eax){1to4}, %ymm6{%k7}	 # AVX512{CD,VL}
	vpconflictq	4064(%edx), %ymm6{%k7}	 # AVX512{CD,VL} Disp8
	vpconflictq	4096(%edx), %ymm6{%k7}	 # AVX512{CD,VL}
	vpconflictq	-4096(%edx), %ymm6{%k7}	 # AVX512{CD,VL} Disp8
	vpconflictq	-4128(%edx), %ymm6{%k7}	 # AVX512{CD,VL}
	vpconflictq	1016(%edx){1to4}, %ymm6{%k7}	 # AVX512{CD,VL} Disp8
	vpconflictq	1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{CD,VL}
	vpconflictq	-1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{CD,VL} Disp8
	vpconflictq	-1032(%edx){1to4}, %ymm6{%k7}	 # AVX512{CD,VL}
	vplzcntd	%xmm5, %xmm6{%k7}	 # AVX512{CD,VL}
	vplzcntd	%xmm5, %xmm6{%k7}{z}	 # AVX512{CD,VL}
	vplzcntd	(%ecx), %xmm6{%k7}	 # AVX512{CD,VL}
	vplzcntd	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{CD,VL}
	vplzcntd	(%eax){1to4}, %xmm6{%k7}	 # AVX512{CD,VL}
	vplzcntd	2032(%edx), %xmm6{%k7}	 # AVX512{CD,VL} Disp8
	vplzcntd	2048(%edx), %xmm6{%k7}	 # AVX512{CD,VL}
	vplzcntd	-2048(%edx), %xmm6{%k7}	 # AVX512{CD,VL} Disp8
	vplzcntd	-2064(%edx), %xmm6{%k7}	 # AVX512{CD,VL}
	vplzcntd	508(%edx){1to4}, %xmm6{%k7}	 # AVX512{CD,VL} Disp8
	vplzcntd	512(%edx){1to4}, %xmm6{%k7}	 # AVX512{CD,VL}
	vplzcntd	-512(%edx){1to4}, %xmm6{%k7}	 # AVX512{CD,VL} Disp8
	vplzcntd	-516(%edx){1to4}, %xmm6{%k7}	 # AVX512{CD,VL}
	vplzcntd	%ymm5, %ymm6{%k7}	 # AVX512{CD,VL}
	vplzcntd	%ymm5, %ymm6{%k7}{z}	 # AVX512{CD,VL}
	vplzcntd	(%ecx), %ymm6{%k7}	 # AVX512{CD,VL}
	vplzcntd	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{CD,VL}
	vplzcntd	(%eax){1to8}, %ymm6{%k7}	 # AVX512{CD,VL}
	vplzcntd	4064(%edx), %ymm6{%k7}	 # AVX512{CD,VL} Disp8
	vplzcntd	4096(%edx), %ymm6{%k7}	 # AVX512{CD,VL}
	vplzcntd	-4096(%edx), %ymm6{%k7}	 # AVX512{CD,VL} Disp8
	vplzcntd	-4128(%edx), %ymm6{%k7}	 # AVX512{CD,VL}
	vplzcntd	508(%edx){1to8}, %ymm6{%k7}	 # AVX512{CD,VL} Disp8
	vplzcntd	512(%edx){1to8}, %ymm6{%k7}	 # AVX512{CD,VL}
	vplzcntd	-512(%edx){1to8}, %ymm6{%k7}	 # AVX512{CD,VL} Disp8
	vplzcntd	-516(%edx){1to8}, %ymm6{%k7}	 # AVX512{CD,VL}
	vplzcntq	%xmm5, %xmm6{%k7}	 # AVX512{CD,VL}
	vplzcntq	%xmm5, %xmm6{%k7}{z}	 # AVX512{CD,VL}
	vplzcntq	(%ecx), %xmm6{%k7}	 # AVX512{CD,VL}
	vplzcntq	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{CD,VL}
	vplzcntq	(%eax){1to2}, %xmm6{%k7}	 # AVX512{CD,VL}
	vplzcntq	2032(%edx), %xmm6{%k7}	 # AVX512{CD,VL} Disp8
	vplzcntq	2048(%edx), %xmm6{%k7}	 # AVX512{CD,VL}
	vplzcntq	-2048(%edx), %xmm6{%k7}	 # AVX512{CD,VL} Disp8
	vplzcntq	-2064(%edx), %xmm6{%k7}	 # AVX512{CD,VL}
	vplzcntq	1016(%edx){1to2}, %xmm6{%k7}	 # AVX512{CD,VL} Disp8
	vplzcntq	1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{CD,VL}
	vplzcntq	-1024(%edx){1to2}, %xmm6{%k7}	 # AVX512{CD,VL} Disp8
	vplzcntq	-1032(%edx){1to2}, %xmm6{%k7}	 # AVX512{CD,VL}
	vplzcntq	%ymm5, %ymm6{%k7}	 # AVX512{CD,VL}
	vplzcntq	%ymm5, %ymm6{%k7}{z}	 # AVX512{CD,VL}
	vplzcntq	(%ecx), %ymm6{%k7}	 # AVX512{CD,VL}
	vplzcntq	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{CD,VL}
	vplzcntq	(%eax){1to4}, %ymm6{%k7}	 # AVX512{CD,VL}
	vplzcntq	4064(%edx), %ymm6{%k7}	 # AVX512{CD,VL} Disp8
	vplzcntq	4096(%edx), %ymm6{%k7}	 # AVX512{CD,VL}
	vplzcntq	-4096(%edx), %ymm6{%k7}	 # AVX512{CD,VL} Disp8
	vplzcntq	-4128(%edx), %ymm6{%k7}	 # AVX512{CD,VL}
	vplzcntq	1016(%edx){1to4}, %ymm6{%k7}	 # AVX512{CD,VL} Disp8
	vplzcntq	1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{CD,VL}
	vplzcntq	-1024(%edx){1to4}, %ymm6{%k7}	 # AVX512{CD,VL} Disp8
	vplzcntq	-1032(%edx){1to4}, %ymm6{%k7}	 # AVX512{CD,VL}
	vpbroadcastmw2d	%k6, %xmm6	 # AVX512{CD,VL}
	vpbroadcastmw2d	%k6, %ymm6	 # AVX512{CD,VL}
	vpbroadcastmb2q	%k6, %xmm6	 # AVX512{CD,VL}
	vpbroadcastmb2q	%k6, %ymm6	 # AVX512{CD,VL}

	.intel_syntax noprefix
	vpconflictd	xmm6{k7}, xmm5	 # AVX512{CD,VL}
	vpconflictd	xmm6{k7}{z}, xmm5	 # AVX512{CD,VL}
	vpconflictd	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{CD,VL}
	vpconflictd	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{CD,VL}
	vpconflictd	xmm6{k7}, [eax]{1to4}	 # AVX512{CD,VL}
	vpconflictd	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{CD,VL} Disp8
	vpconflictd	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{CD,VL}
	vpconflictd	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{CD,VL} Disp8
	vpconflictd	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{CD,VL}
	vpconflictd	xmm6{k7}, [edx+508]{1to4}	 # AVX512{CD,VL} Disp8
	vpconflictd	xmm6{k7}, [edx+512]{1to4}	 # AVX512{CD,VL}
	vpconflictd	xmm6{k7}, [edx-512]{1to4}	 # AVX512{CD,VL} Disp8
	vpconflictd	xmm6{k7}, [edx-516]{1to4}	 # AVX512{CD,VL}
	vpconflictd	ymm6{k7}, ymm5	 # AVX512{CD,VL}
	vpconflictd	ymm6{k7}{z}, ymm5	 # AVX512{CD,VL}
	vpconflictd	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{CD,VL}
	vpconflictd	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{CD,VL}
	vpconflictd	ymm6{k7}, [eax]{1to8}	 # AVX512{CD,VL}
	vpconflictd	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{CD,VL} Disp8
	vpconflictd	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{CD,VL}
	vpconflictd	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{CD,VL} Disp8
	vpconflictd	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{CD,VL}
	vpconflictd	ymm6{k7}, [edx+508]{1to8}	 # AVX512{CD,VL} Disp8
	vpconflictd	ymm6{k7}, [edx+512]{1to8}	 # AVX512{CD,VL}
	vpconflictd	ymm6{k7}, [edx-512]{1to8}	 # AVX512{CD,VL} Disp8
	vpconflictd	ymm6{k7}, [edx-516]{1to8}	 # AVX512{CD,VL}
	vpconflictq	xmm6{k7}, xmm5	 # AVX512{CD,VL}
	vpconflictq	xmm6{k7}{z}, xmm5	 # AVX512{CD,VL}
	vpconflictq	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{CD,VL}
	vpconflictq	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{CD,VL}
	vpconflictq	xmm6{k7}, [eax]{1to2}	 # AVX512{CD,VL}
	vpconflictq	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{CD,VL} Disp8
	vpconflictq	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{CD,VL}
	vpconflictq	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{CD,VL} Disp8
	vpconflictq	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{CD,VL}
	vpconflictq	xmm6{k7}, [edx+1016]{1to2}	 # AVX512{CD,VL} Disp8
	vpconflictq	xmm6{k7}, [edx+1024]{1to2}	 # AVX512{CD,VL}
	vpconflictq	xmm6{k7}, [edx-1024]{1to2}	 # AVX512{CD,VL} Disp8
	vpconflictq	xmm6{k7}, [edx-1032]{1to2}	 # AVX512{CD,VL}
	vpconflictq	ymm6{k7}, ymm5	 # AVX512{CD,VL}
	vpconflictq	ymm6{k7}{z}, ymm5	 # AVX512{CD,VL}
	vpconflictq	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{CD,VL}
	vpconflictq	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{CD,VL}
	vpconflictq	ymm6{k7}, [eax]{1to4}	 # AVX512{CD,VL}
	vpconflictq	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{CD,VL} Disp8
	vpconflictq	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{CD,VL}
	vpconflictq	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{CD,VL} Disp8
	vpconflictq	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{CD,VL}
	vpconflictq	ymm6{k7}, [edx+1016]{1to4}	 # AVX512{CD,VL} Disp8
	vpconflictq	ymm6{k7}, [edx+1024]{1to4}	 # AVX512{CD,VL}
	vpconflictq	ymm6{k7}, [edx-1024]{1to4}	 # AVX512{CD,VL} Disp8
	vpconflictq	ymm6{k7}, [edx-1032]{1to4}	 # AVX512{CD,VL}
	vplzcntd	xmm6{k7}, xmm5	 # AVX512{CD,VL}
	vplzcntd	xmm6{k7}{z}, xmm5	 # AVX512{CD,VL}
	vplzcntd	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{CD,VL}
	vplzcntd	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{CD,VL}
	vplzcntd	xmm6{k7}, [eax]{1to4}	 # AVX512{CD,VL}
	vplzcntd	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{CD,VL} Disp8
	vplzcntd	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{CD,VL}
	vplzcntd	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{CD,VL} Disp8
	vplzcntd	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{CD,VL}
	vplzcntd	xmm6{k7}, [edx+508]{1to4}	 # AVX512{CD,VL} Disp8
	vplzcntd	xmm6{k7}, [edx+512]{1to4}	 # AVX512{CD,VL}
	vplzcntd	xmm6{k7}, [edx-512]{1to4}	 # AVX512{CD,VL} Disp8
	vplzcntd	xmm6{k7}, [edx-516]{1to4}	 # AVX512{CD,VL}
	vplzcntd	ymm6{k7}, ymm5	 # AVX512{CD,VL}
	vplzcntd	ymm6{k7}{z}, ymm5	 # AVX512{CD,VL}
	vplzcntd	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{CD,VL}
	vplzcntd	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{CD,VL}
	vplzcntd	ymm6{k7}, [eax]{1to8}	 # AVX512{CD,VL}
	vplzcntd	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{CD,VL} Disp8
	vplzcntd	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{CD,VL}
	vplzcntd	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{CD,VL} Disp8
	vplzcntd	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{CD,VL}
	vplzcntd	ymm6{k7}, [edx+508]{1to8}	 # AVX512{CD,VL} Disp8
	vplzcntd	ymm6{k7}, [edx+512]{1to8}	 # AVX512{CD,VL}
	vplzcntd	ymm6{k7}, [edx-512]{1to8}	 # AVX512{CD,VL} Disp8
	vplzcntd	ymm6{k7}, [edx-516]{1to8}	 # AVX512{CD,VL}
	vplzcntq	xmm6{k7}, xmm5	 # AVX512{CD,VL}
	vplzcntq	xmm6{k7}{z}, xmm5	 # AVX512{CD,VL}
	vplzcntq	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{CD,VL}
	vplzcntq	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{CD,VL}
	vplzcntq	xmm6{k7}, [eax]{1to2}	 # AVX512{CD,VL}
	vplzcntq	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{CD,VL} Disp8
	vplzcntq	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{CD,VL}
	vplzcntq	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{CD,VL} Disp8
	vplzcntq	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{CD,VL}
	vplzcntq	xmm6{k7}, [edx+1016]{1to2}	 # AVX512{CD,VL} Disp8
	vplzcntq	xmm6{k7}, [edx+1024]{1to2}	 # AVX512{CD,VL}
	vplzcntq	xmm6{k7}, [edx-1024]{1to2}	 # AVX512{CD,VL} Disp8
	vplzcntq	xmm6{k7}, [edx-1032]{1to2}	 # AVX512{CD,VL}
	vplzcntq	ymm6{k7}, ymm5	 # AVX512{CD,VL}
	vplzcntq	ymm6{k7}{z}, ymm5	 # AVX512{CD,VL}
	vplzcntq	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{CD,VL}
	vplzcntq	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{CD,VL}
	vplzcntq	ymm6{k7}, [eax]{1to4}	 # AVX512{CD,VL}
	vplzcntq	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{CD,VL} Disp8
	vplzcntq	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{CD,VL}
	vplzcntq	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{CD,VL} Disp8
	vplzcntq	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{CD,VL}
	vplzcntq	ymm6{k7}, [edx+1016]{1to4}	 # AVX512{CD,VL} Disp8
	vplzcntq	ymm6{k7}, [edx+1024]{1to4}	 # AVX512{CD,VL}
	vplzcntq	ymm6{k7}, [edx-1024]{1to4}	 # AVX512{CD,VL} Disp8
	vplzcntq	ymm6{k7}, [edx-1032]{1to4}	 # AVX512{CD,VL}
	vpbroadcastmw2d	xmm6, k6	 # AVX512{CD,VL}
	vpbroadcastmw2d	ymm6, k6	 # AVX512{CD,VL}
	vpbroadcastmb2q	xmm6, k6	 # AVX512{CD,VL}
	vpbroadcastmb2q	ymm6, k6	 # AVX512{CD,VL}
