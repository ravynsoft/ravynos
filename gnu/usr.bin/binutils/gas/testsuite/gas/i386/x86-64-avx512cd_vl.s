# Check 64bit AVX512{CD,VL} instructions

	.allow_index_reg
	.text
_start:
	vpconflictd	%xmm29, %xmm30	 # AVX512{CD,VL}
	vpconflictd	%xmm29, %xmm30{%k7}	 # AVX512{CD,VL}
	vpconflictd	%xmm29, %xmm30{%k7}{z}	 # AVX512{CD,VL}
	vpconflictd	(%rcx), %xmm30	 # AVX512{CD,VL}
	vpconflictd	0x123(%rax,%r14,8), %xmm30	 # AVX512{CD,VL}
	vpconflictd	(%rcx){1to4}, %xmm30	 # AVX512{CD,VL}
	vpconflictd	2032(%rdx), %xmm30	 # AVX512{CD,VL} Disp8
	vpconflictd	2048(%rdx), %xmm30	 # AVX512{CD,VL}
	vpconflictd	-2048(%rdx), %xmm30	 # AVX512{CD,VL} Disp8
	vpconflictd	-2064(%rdx), %xmm30	 # AVX512{CD,VL}
	vpconflictd	508(%rdx){1to4}, %xmm30	 # AVX512{CD,VL} Disp8
	vpconflictd	512(%rdx){1to4}, %xmm30	 # AVX512{CD,VL}
	vpconflictd	-512(%rdx){1to4}, %xmm30	 # AVX512{CD,VL} Disp8
	vpconflictd	-516(%rdx){1to4}, %xmm30	 # AVX512{CD,VL}
	vpconflictd	%ymm29, %ymm30	 # AVX512{CD,VL}
	vpconflictd	%ymm29, %ymm30{%k7}	 # AVX512{CD,VL}
	vpconflictd	%ymm29, %ymm30{%k7}{z}	 # AVX512{CD,VL}
	vpconflictd	(%rcx), %ymm30	 # AVX512{CD,VL}
	vpconflictd	0x123(%rax,%r14,8), %ymm30	 # AVX512{CD,VL}
	vpconflictd	(%rcx){1to8}, %ymm30	 # AVX512{CD,VL}
	vpconflictd	4064(%rdx), %ymm30	 # AVX512{CD,VL} Disp8
	vpconflictd	4096(%rdx), %ymm30	 # AVX512{CD,VL}
	vpconflictd	-4096(%rdx), %ymm30	 # AVX512{CD,VL} Disp8
	vpconflictd	-4128(%rdx), %ymm30	 # AVX512{CD,VL}
	vpconflictd	508(%rdx){1to8}, %ymm30	 # AVX512{CD,VL} Disp8
	vpconflictd	512(%rdx){1to8}, %ymm30	 # AVX512{CD,VL}
	vpconflictd	-512(%rdx){1to8}, %ymm30	 # AVX512{CD,VL} Disp8
	vpconflictd	-516(%rdx){1to8}, %ymm30	 # AVX512{CD,VL}
	vpconflictq	%xmm29, %xmm30	 # AVX512{CD,VL}
	vpconflictq	%xmm29, %xmm30{%k7}	 # AVX512{CD,VL}
	vpconflictq	%xmm29, %xmm30{%k7}{z}	 # AVX512{CD,VL}
	vpconflictq	(%rcx), %xmm30	 # AVX512{CD,VL}
	vpconflictq	0x123(%rax,%r14,8), %xmm30	 # AVX512{CD,VL}
	vpconflictq	(%rcx){1to2}, %xmm30	 # AVX512{CD,VL}
	vpconflictq	2032(%rdx), %xmm30	 # AVX512{CD,VL} Disp8
	vpconflictq	2048(%rdx), %xmm30	 # AVX512{CD,VL}
	vpconflictq	-2048(%rdx), %xmm30	 # AVX512{CD,VL} Disp8
	vpconflictq	-2064(%rdx), %xmm30	 # AVX512{CD,VL}
	vpconflictq	1016(%rdx){1to2}, %xmm30	 # AVX512{CD,VL} Disp8
	vpconflictq	1024(%rdx){1to2}, %xmm30	 # AVX512{CD,VL}
	vpconflictq	-1024(%rdx){1to2}, %xmm30	 # AVX512{CD,VL} Disp8
	vpconflictq	-1032(%rdx){1to2}, %xmm30	 # AVX512{CD,VL}
	vpconflictq	%ymm29, %ymm30	 # AVX512{CD,VL}
	vpconflictq	%ymm29, %ymm30{%k7}	 # AVX512{CD,VL}
	vpconflictq	%ymm29, %ymm30{%k7}{z}	 # AVX512{CD,VL}
	vpconflictq	(%rcx), %ymm30	 # AVX512{CD,VL}
	vpconflictq	0x123(%rax,%r14,8), %ymm30	 # AVX512{CD,VL}
	vpconflictq	(%rcx){1to4}, %ymm30	 # AVX512{CD,VL}
	vpconflictq	4064(%rdx), %ymm30	 # AVX512{CD,VL} Disp8
	vpconflictq	4096(%rdx), %ymm30	 # AVX512{CD,VL}
	vpconflictq	-4096(%rdx), %ymm30	 # AVX512{CD,VL} Disp8
	vpconflictq	-4128(%rdx), %ymm30	 # AVX512{CD,VL}
	vpconflictq	1016(%rdx){1to4}, %ymm30	 # AVX512{CD,VL} Disp8
	vpconflictq	1024(%rdx){1to4}, %ymm30	 # AVX512{CD,VL}
	vpconflictq	-1024(%rdx){1to4}, %ymm30	 # AVX512{CD,VL} Disp8
	vpconflictq	-1032(%rdx){1to4}, %ymm30	 # AVX512{CD,VL}
	vplzcntd	%xmm29, %xmm30	 # AVX512{CD,VL}
	vplzcntd	%xmm29, %xmm30{%k7}	 # AVX512{CD,VL}
	vplzcntd	%xmm29, %xmm30{%k7}{z}	 # AVX512{CD,VL}
	vplzcntd	(%rcx), %xmm30	 # AVX512{CD,VL}
	vplzcntd	0x123(%rax,%r14,8), %xmm30	 # AVX512{CD,VL}
	vplzcntd	(%rcx){1to4}, %xmm30	 # AVX512{CD,VL}
	vplzcntd	2032(%rdx), %xmm30	 # AVX512{CD,VL} Disp8
	vplzcntd	2048(%rdx), %xmm30	 # AVX512{CD,VL}
	vplzcntd	-2048(%rdx), %xmm30	 # AVX512{CD,VL} Disp8
	vplzcntd	-2064(%rdx), %xmm30	 # AVX512{CD,VL}
	vplzcntd	508(%rdx){1to4}, %xmm30	 # AVX512{CD,VL} Disp8
	vplzcntd	512(%rdx){1to4}, %xmm30	 # AVX512{CD,VL}
	vplzcntd	-512(%rdx){1to4}, %xmm30	 # AVX512{CD,VL} Disp8
	vplzcntd	-516(%rdx){1to4}, %xmm30	 # AVX512{CD,VL}
	vplzcntd	%ymm29, %ymm30	 # AVX512{CD,VL}
	vplzcntd	%ymm29, %ymm30{%k7}	 # AVX512{CD,VL}
	vplzcntd	%ymm29, %ymm30{%k7}{z}	 # AVX512{CD,VL}
	vplzcntd	(%rcx), %ymm30	 # AVX512{CD,VL}
	vplzcntd	0x123(%rax,%r14,8), %ymm30	 # AVX512{CD,VL}
	vplzcntd	(%rcx){1to8}, %ymm30	 # AVX512{CD,VL}
	vplzcntd	4064(%rdx), %ymm30	 # AVX512{CD,VL} Disp8
	vplzcntd	4096(%rdx), %ymm30	 # AVX512{CD,VL}
	vplzcntd	-4096(%rdx), %ymm30	 # AVX512{CD,VL} Disp8
	vplzcntd	-4128(%rdx), %ymm30	 # AVX512{CD,VL}
	vplzcntd	508(%rdx){1to8}, %ymm30	 # AVX512{CD,VL} Disp8
	vplzcntd	512(%rdx){1to8}, %ymm30	 # AVX512{CD,VL}
	vplzcntd	-512(%rdx){1to8}, %ymm30	 # AVX512{CD,VL} Disp8
	vplzcntd	-516(%rdx){1to8}, %ymm30	 # AVX512{CD,VL}
	vplzcntq	%xmm29, %xmm30	 # AVX512{CD,VL}
	vplzcntq	%xmm29, %xmm30{%k7}	 # AVX512{CD,VL}
	vplzcntq	%xmm29, %xmm30{%k7}{z}	 # AVX512{CD,VL}
	vplzcntq	(%rcx), %xmm30	 # AVX512{CD,VL}
	vplzcntq	0x123(%rax,%r14,8), %xmm30	 # AVX512{CD,VL}
	vplzcntq	(%rcx){1to2}, %xmm30	 # AVX512{CD,VL}
	vplzcntq	2032(%rdx), %xmm30	 # AVX512{CD,VL} Disp8
	vplzcntq	2048(%rdx), %xmm30	 # AVX512{CD,VL}
	vplzcntq	-2048(%rdx), %xmm30	 # AVX512{CD,VL} Disp8
	vplzcntq	-2064(%rdx), %xmm30	 # AVX512{CD,VL}
	vplzcntq	1016(%rdx){1to2}, %xmm30	 # AVX512{CD,VL} Disp8
	vplzcntq	1024(%rdx){1to2}, %xmm30	 # AVX512{CD,VL}
	vplzcntq	-1024(%rdx){1to2}, %xmm30	 # AVX512{CD,VL} Disp8
	vplzcntq	-1032(%rdx){1to2}, %xmm30	 # AVX512{CD,VL}
	vplzcntq	%ymm29, %ymm30	 # AVX512{CD,VL}
	vplzcntq	%ymm29, %ymm30{%k7}	 # AVX512{CD,VL}
	vplzcntq	%ymm29, %ymm30{%k7}{z}	 # AVX512{CD,VL}
	vplzcntq	(%rcx), %ymm30	 # AVX512{CD,VL}
	vplzcntq	0x123(%rax,%r14,8), %ymm30	 # AVX512{CD,VL}
	vplzcntq	(%rcx){1to4}, %ymm30	 # AVX512{CD,VL}
	vplzcntq	4064(%rdx), %ymm30	 # AVX512{CD,VL} Disp8
	vplzcntq	4096(%rdx), %ymm30	 # AVX512{CD,VL}
	vplzcntq	-4096(%rdx), %ymm30	 # AVX512{CD,VL} Disp8
	vplzcntq	-4128(%rdx), %ymm30	 # AVX512{CD,VL}
	vplzcntq	1016(%rdx){1to4}, %ymm30	 # AVX512{CD,VL} Disp8
	vplzcntq	1024(%rdx){1to4}, %ymm30	 # AVX512{CD,VL}
	vplzcntq	-1024(%rdx){1to4}, %ymm30	 # AVX512{CD,VL} Disp8
	vplzcntq	-1032(%rdx){1to4}, %ymm30	 # AVX512{CD,VL}
	vpbroadcastmw2d	%k6, %xmm30	 # AVX512{CD,VL}
	vpbroadcastmw2d	%k6, %ymm30	 # AVX512{CD,VL}
	vpbroadcastmb2q	%k6, %xmm30	 # AVX512{CD,VL}
	vpbroadcastmb2q	%k6, %ymm30	 # AVX512{CD,VL}

	.intel_syntax noprefix
	vpconflictd	xmm30, xmm29	 # AVX512{CD,VL}
	vpconflictd	xmm30{k7}, xmm29	 # AVX512{CD,VL}
	vpconflictd	xmm30{k7}{z}, xmm29	 # AVX512{CD,VL}
	vpconflictd	xmm30, XMMWORD PTR [rcx]	 # AVX512{CD,VL}
	vpconflictd	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{CD,VL}
	vpconflictd	xmm30, [rcx]{1to4}	 # AVX512{CD,VL}
	vpconflictd	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{CD,VL} Disp8
	vpconflictd	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{CD,VL}
	vpconflictd	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{CD,VL} Disp8
	vpconflictd	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{CD,VL}
	vpconflictd	xmm30, [rdx+508]{1to4}	 # AVX512{CD,VL} Disp8
	vpconflictd	xmm30, [rdx+512]{1to4}	 # AVX512{CD,VL}
	vpconflictd	xmm30, [rdx-512]{1to4}	 # AVX512{CD,VL} Disp8
	vpconflictd	xmm30, [rdx-516]{1to4}	 # AVX512{CD,VL}
	vpconflictd	ymm30, ymm29	 # AVX512{CD,VL}
	vpconflictd	ymm30{k7}, ymm29	 # AVX512{CD,VL}
	vpconflictd	ymm30{k7}{z}, ymm29	 # AVX512{CD,VL}
	vpconflictd	ymm30, YMMWORD PTR [rcx]	 # AVX512{CD,VL}
	vpconflictd	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{CD,VL}
	vpconflictd	ymm30, [rcx]{1to8}	 # AVX512{CD,VL}
	vpconflictd	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{CD,VL} Disp8
	vpconflictd	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{CD,VL}
	vpconflictd	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{CD,VL} Disp8
	vpconflictd	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{CD,VL}
	vpconflictd	ymm30, [rdx+508]{1to8}	 # AVX512{CD,VL} Disp8
	vpconflictd	ymm30, [rdx+512]{1to8}	 # AVX512{CD,VL}
	vpconflictd	ymm30, [rdx-512]{1to8}	 # AVX512{CD,VL} Disp8
	vpconflictd	ymm30, [rdx-516]{1to8}	 # AVX512{CD,VL}
	vpconflictq	xmm30, xmm29	 # AVX512{CD,VL}
	vpconflictq	xmm30{k7}, xmm29	 # AVX512{CD,VL}
	vpconflictq	xmm30{k7}{z}, xmm29	 # AVX512{CD,VL}
	vpconflictq	xmm30, XMMWORD PTR [rcx]	 # AVX512{CD,VL}
	vpconflictq	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{CD,VL}
	vpconflictq	xmm30, [rcx]{1to2}	 # AVX512{CD,VL}
	vpconflictq	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{CD,VL} Disp8
	vpconflictq	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{CD,VL}
	vpconflictq	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{CD,VL} Disp8
	vpconflictq	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{CD,VL}
	vpconflictq	xmm30, [rdx+1016]{1to2}	 # AVX512{CD,VL} Disp8
	vpconflictq	xmm30, [rdx+1024]{1to2}	 # AVX512{CD,VL}
	vpconflictq	xmm30, [rdx-1024]{1to2}	 # AVX512{CD,VL} Disp8
	vpconflictq	xmm30, [rdx-1032]{1to2}	 # AVX512{CD,VL}
	vpconflictq	ymm30, ymm29	 # AVX512{CD,VL}
	vpconflictq	ymm30{k7}, ymm29	 # AVX512{CD,VL}
	vpconflictq	ymm30{k7}{z}, ymm29	 # AVX512{CD,VL}
	vpconflictq	ymm30, YMMWORD PTR [rcx]	 # AVX512{CD,VL}
	vpconflictq	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{CD,VL}
	vpconflictq	ymm30, [rcx]{1to4}	 # AVX512{CD,VL}
	vpconflictq	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{CD,VL} Disp8
	vpconflictq	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{CD,VL}
	vpconflictq	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{CD,VL} Disp8
	vpconflictq	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{CD,VL}
	vpconflictq	ymm30, [rdx+1016]{1to4}	 # AVX512{CD,VL} Disp8
	vpconflictq	ymm30, [rdx+1024]{1to4}	 # AVX512{CD,VL}
	vpconflictq	ymm30, [rdx-1024]{1to4}	 # AVX512{CD,VL} Disp8
	vpconflictq	ymm30, [rdx-1032]{1to4}	 # AVX512{CD,VL}
	vplzcntd	xmm30, xmm29	 # AVX512{CD,VL}
	vplzcntd	xmm30{k7}, xmm29	 # AVX512{CD,VL}
	vplzcntd	xmm30{k7}{z}, xmm29	 # AVX512{CD,VL}
	vplzcntd	xmm30, XMMWORD PTR [rcx]	 # AVX512{CD,VL}
	vplzcntd	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{CD,VL}
	vplzcntd	xmm30, [rcx]{1to4}	 # AVX512{CD,VL}
	vplzcntd	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{CD,VL} Disp8
	vplzcntd	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{CD,VL}
	vplzcntd	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{CD,VL} Disp8
	vplzcntd	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{CD,VL}
	vplzcntd	xmm30, [rdx+508]{1to4}	 # AVX512{CD,VL} Disp8
	vplzcntd	xmm30, [rdx+512]{1to4}	 # AVX512{CD,VL}
	vplzcntd	xmm30, [rdx-512]{1to4}	 # AVX512{CD,VL} Disp8
	vplzcntd	xmm30, [rdx-516]{1to4}	 # AVX512{CD,VL}
	vplzcntd	ymm30, ymm29	 # AVX512{CD,VL}
	vplzcntd	ymm30{k7}, ymm29	 # AVX512{CD,VL}
	vplzcntd	ymm30{k7}{z}, ymm29	 # AVX512{CD,VL}
	vplzcntd	ymm30, YMMWORD PTR [rcx]	 # AVX512{CD,VL}
	vplzcntd	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{CD,VL}
	vplzcntd	ymm30, [rcx]{1to8}	 # AVX512{CD,VL}
	vplzcntd	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{CD,VL} Disp8
	vplzcntd	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{CD,VL}
	vplzcntd	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{CD,VL} Disp8
	vplzcntd	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{CD,VL}
	vplzcntd	ymm30, [rdx+508]{1to8}	 # AVX512{CD,VL} Disp8
	vplzcntd	ymm30, [rdx+512]{1to8}	 # AVX512{CD,VL}
	vplzcntd	ymm30, [rdx-512]{1to8}	 # AVX512{CD,VL} Disp8
	vplzcntd	ymm30, [rdx-516]{1to8}	 # AVX512{CD,VL}
	vplzcntq	xmm30, xmm29	 # AVX512{CD,VL}
	vplzcntq	xmm30{k7}, xmm29	 # AVX512{CD,VL}
	vplzcntq	xmm30{k7}{z}, xmm29	 # AVX512{CD,VL}
	vplzcntq	xmm30, XMMWORD PTR [rcx]	 # AVX512{CD,VL}
	vplzcntq	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{CD,VL}
	vplzcntq	xmm30, [rcx]{1to2}	 # AVX512{CD,VL}
	vplzcntq	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{CD,VL} Disp8
	vplzcntq	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{CD,VL}
	vplzcntq	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{CD,VL} Disp8
	vplzcntq	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{CD,VL}
	vplzcntq	xmm30, [rdx+1016]{1to2}	 # AVX512{CD,VL} Disp8
	vplzcntq	xmm30, [rdx+1024]{1to2}	 # AVX512{CD,VL}
	vplzcntq	xmm30, [rdx-1024]{1to2}	 # AVX512{CD,VL} Disp8
	vplzcntq	xmm30, [rdx-1032]{1to2}	 # AVX512{CD,VL}
	vplzcntq	ymm30, ymm29	 # AVX512{CD,VL}
	vplzcntq	ymm30{k7}, ymm29	 # AVX512{CD,VL}
	vplzcntq	ymm30{k7}{z}, ymm29	 # AVX512{CD,VL}
	vplzcntq	ymm30, YMMWORD PTR [rcx]	 # AVX512{CD,VL}
	vplzcntq	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{CD,VL}
	vplzcntq	ymm30, [rcx]{1to4}	 # AVX512{CD,VL}
	vplzcntq	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{CD,VL} Disp8
	vplzcntq	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{CD,VL}
	vplzcntq	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{CD,VL} Disp8
	vplzcntq	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{CD,VL}
	vplzcntq	ymm30, [rdx+1016]{1to4}	 # AVX512{CD,VL} Disp8
	vplzcntq	ymm30, [rdx+1024]{1to4}	 # AVX512{CD,VL}
	vplzcntq	ymm30, [rdx-1024]{1to4}	 # AVX512{CD,VL} Disp8
	vplzcntq	ymm30, [rdx-1032]{1to4}	 # AVX512{CD,VL}
	vpbroadcastmw2d	xmm30, k6	 # AVX512{CD,VL}
	vpbroadcastmw2d	ymm30, k6	 # AVX512{CD,VL}
	vpbroadcastmb2q	xmm30, k6	 # AVX512{CD,VL}
	vpbroadcastmb2q	ymm30, k6	 # AVX512{CD,VL}
