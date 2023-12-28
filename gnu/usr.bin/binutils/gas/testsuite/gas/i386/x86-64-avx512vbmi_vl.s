# Check 64bit AVX512{VBMI,VL} instructions

	.allow_index_reg
	.text
_start:
	vpermb	%xmm28, %xmm29, %xmm30	 # AVX512{VBMI,VL}
	vpermb	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{VBMI,VL}
	vpermb	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{VBMI,VL}
	vpermb	(%rcx), %xmm29, %xmm30	 # AVX512{VBMI,VL}
	vpermb	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{VBMI,VL}
	vpermb	2032(%rdx), %xmm29, %xmm30	 # AVX512{VBMI,VL} Disp8
	vpermb	2048(%rdx), %xmm29, %xmm30	 # AVX512{VBMI,VL}
	vpermb	-2048(%rdx), %xmm29, %xmm30	 # AVX512{VBMI,VL} Disp8
	vpermb	-2064(%rdx), %xmm29, %xmm30	 # AVX512{VBMI,VL}
	vpermb	%ymm28, %ymm29, %ymm30	 # AVX512{VBMI,VL}
	vpermb	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{VBMI,VL}
	vpermb	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{VBMI,VL}
	vpermb	(%rcx), %ymm29, %ymm30	 # AVX512{VBMI,VL}
	vpermb	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{VBMI,VL}
	vpermb	4064(%rdx), %ymm29, %ymm30	 # AVX512{VBMI,VL} Disp8
	vpermb	4096(%rdx), %ymm29, %ymm30	 # AVX512{VBMI,VL}
	vpermb	-4096(%rdx), %ymm29, %ymm30	 # AVX512{VBMI,VL} Disp8
	vpermb	-4128(%rdx), %ymm29, %ymm30	 # AVX512{VBMI,VL}
	vpermi2b	%xmm28, %xmm29, %xmm30	 # AVX512{VBMI,VL}
	vpermi2b	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{VBMI,VL}
	vpermi2b	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{VBMI,VL}
	vpermi2b	(%rcx), %xmm29, %xmm30	 # AVX512{VBMI,VL}
	vpermi2b	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{VBMI,VL}
	vpermi2b	2032(%rdx), %xmm29, %xmm30	 # AVX512{VBMI,VL} Disp8
	vpermi2b	2048(%rdx), %xmm29, %xmm30	 # AVX512{VBMI,VL}
	vpermi2b	-2048(%rdx), %xmm29, %xmm30	 # AVX512{VBMI,VL} Disp8
	vpermi2b	-2064(%rdx), %xmm29, %xmm30	 # AVX512{VBMI,VL}
	vpermi2b	%ymm28, %ymm29, %ymm30	 # AVX512{VBMI,VL}
	vpermi2b	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{VBMI,VL}
	vpermi2b	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{VBMI,VL}
	vpermi2b	(%rcx), %ymm29, %ymm30	 # AVX512{VBMI,VL}
	vpermi2b	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{VBMI,VL}
	vpermi2b	4064(%rdx), %ymm29, %ymm30	 # AVX512{VBMI,VL} Disp8
	vpermi2b	4096(%rdx), %ymm29, %ymm30	 # AVX512{VBMI,VL}
	vpermi2b	-4096(%rdx), %ymm29, %ymm30	 # AVX512{VBMI,VL} Disp8
	vpermi2b	-4128(%rdx), %ymm29, %ymm30	 # AVX512{VBMI,VL}
	vpermt2b	%xmm28, %xmm29, %xmm30	 # AVX512{VBMI,VL}
	vpermt2b	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{VBMI,VL}
	vpermt2b	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{VBMI,VL}
	vpermt2b	(%rcx), %xmm29, %xmm30	 # AVX512{VBMI,VL}
	vpermt2b	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{VBMI,VL}
	vpermt2b	2032(%rdx), %xmm29, %xmm30	 # AVX512{VBMI,VL} Disp8
	vpermt2b	2048(%rdx), %xmm29, %xmm30	 # AVX512{VBMI,VL}
	vpermt2b	-2048(%rdx), %xmm29, %xmm30	 # AVX512{VBMI,VL} Disp8
	vpermt2b	-2064(%rdx), %xmm29, %xmm30	 # AVX512{VBMI,VL}
	vpermt2b	%ymm28, %ymm29, %ymm30	 # AVX512{VBMI,VL}
	vpermt2b	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{VBMI,VL}
	vpermt2b	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{VBMI,VL}
	vpermt2b	(%rcx), %ymm29, %ymm30	 # AVX512{VBMI,VL}
	vpermt2b	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{VBMI,VL}
	vpermt2b	4064(%rdx), %ymm29, %ymm30	 # AVX512{VBMI,VL} Disp8
	vpermt2b	4096(%rdx), %ymm29, %ymm30	 # AVX512{VBMI,VL}
	vpermt2b	-4096(%rdx), %ymm29, %ymm30	 # AVX512{VBMI,VL} Disp8
	vpermt2b	-4128(%rdx), %ymm29, %ymm30	 # AVX512{VBMI,VL}
	vpmultishiftqb	%xmm28, %xmm29, %xmm30	 # AVX512{VBMI,VL}
	vpmultishiftqb	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{VBMI,VL}
	vpmultishiftqb	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{VBMI,VL}
	vpmultishiftqb	(%rcx), %xmm29, %xmm30	 # AVX512{VBMI,VL}
	vpmultishiftqb	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{VBMI,VL}
	vpmultishiftqb	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{VBMI,VL}
	vpmultishiftqb	2032(%rdx), %xmm29, %xmm30	 # AVX512{VBMI,VL} Disp8
	vpmultishiftqb	2048(%rdx), %xmm29, %xmm30	 # AVX512{VBMI,VL}
	vpmultishiftqb	-2048(%rdx), %xmm29, %xmm30	 # AVX512{VBMI,VL} Disp8
	vpmultishiftqb	-2064(%rdx), %xmm29, %xmm30	 # AVX512{VBMI,VL}
	vpmultishiftqb	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{VBMI,VL} Disp8
	vpmultishiftqb	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{VBMI,VL}
	vpmultishiftqb	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{VBMI,VL} Disp8
	vpmultishiftqb	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{VBMI,VL}
	vpmultishiftqb	%ymm28, %ymm29, %ymm30	 # AVX512{VBMI,VL}
	vpmultishiftqb	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{VBMI,VL}
	vpmultishiftqb	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{VBMI,VL}
	vpmultishiftqb	(%rcx), %ymm29, %ymm30	 # AVX512{VBMI,VL}
	vpmultishiftqb	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{VBMI,VL}
	vpmultishiftqb	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{VBMI,VL}
	vpmultishiftqb	4064(%rdx), %ymm29, %ymm30	 # AVX512{VBMI,VL} Disp8
	vpmultishiftqb	4096(%rdx), %ymm29, %ymm30	 # AVX512{VBMI,VL}
	vpmultishiftqb	-4096(%rdx), %ymm29, %ymm30	 # AVX512{VBMI,VL} Disp8
	vpmultishiftqb	-4128(%rdx), %ymm29, %ymm30	 # AVX512{VBMI,VL}
	vpmultishiftqb	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{VBMI,VL} Disp8
	vpmultishiftqb	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{VBMI,VL}
	vpmultishiftqb	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{VBMI,VL} Disp8
	vpmultishiftqb	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{VBMI,VL}

	.intel_syntax noprefix
	vpermb	xmm30, xmm29, xmm28	 # AVX512{VBMI,VL}
	vpermb	xmm30{k7}, xmm29, xmm28	 # AVX512{VBMI,VL}
	vpermb	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{VBMI,VL}
	vpermb	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{VBMI,VL}
	vpermb	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{VBMI,VL}
	vpermb	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{VBMI,VL} Disp8
	vpermb	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{VBMI,VL}
	vpermb	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{VBMI,VL} Disp8
	vpermb	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{VBMI,VL}
	vpermb	ymm30, ymm29, ymm28	 # AVX512{VBMI,VL}
	vpermb	ymm30{k7}, ymm29, ymm28	 # AVX512{VBMI,VL}
	vpermb	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{VBMI,VL}
	vpermb	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{VBMI,VL}
	vpermb	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{VBMI,VL}
	vpermb	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{VBMI,VL} Disp8
	vpermb	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{VBMI,VL}
	vpermb	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{VBMI,VL} Disp8
	vpermb	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{VBMI,VL}
	vpermi2b	xmm30, xmm29, xmm28	 # AVX512{VBMI,VL}
	vpermi2b	xmm30{k7}, xmm29, xmm28	 # AVX512{VBMI,VL}
	vpermi2b	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{VBMI,VL}
	vpermi2b	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{VBMI,VL}
	vpermi2b	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{VBMI,VL}
	vpermi2b	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{VBMI,VL} Disp8
	vpermi2b	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{VBMI,VL}
	vpermi2b	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{VBMI,VL} Disp8
	vpermi2b	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{VBMI,VL}
	vpermi2b	ymm30, ymm29, ymm28	 # AVX512{VBMI,VL}
	vpermi2b	ymm30{k7}, ymm29, ymm28	 # AVX512{VBMI,VL}
	vpermi2b	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{VBMI,VL}
	vpermi2b	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{VBMI,VL}
	vpermi2b	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{VBMI,VL}
	vpermi2b	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{VBMI,VL} Disp8
	vpermi2b	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{VBMI,VL}
	vpermi2b	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{VBMI,VL} Disp8
	vpermi2b	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{VBMI,VL}
	vpermt2b	xmm30, xmm29, xmm28	 # AVX512{VBMI,VL}
	vpermt2b	xmm30{k7}, xmm29, xmm28	 # AVX512{VBMI,VL}
	vpermt2b	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{VBMI,VL}
	vpermt2b	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{VBMI,VL}
	vpermt2b	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{VBMI,VL}
	vpermt2b	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{VBMI,VL} Disp8
	vpermt2b	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{VBMI,VL}
	vpermt2b	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{VBMI,VL} Disp8
	vpermt2b	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{VBMI,VL}
	vpermt2b	ymm30, ymm29, ymm28	 # AVX512{VBMI,VL}
	vpermt2b	ymm30{k7}, ymm29, ymm28	 # AVX512{VBMI,VL}
	vpermt2b	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{VBMI,VL}
	vpermt2b	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{VBMI,VL}
	vpermt2b	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{VBMI,VL}
	vpermt2b	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{VBMI,VL} Disp8
	vpermt2b	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{VBMI,VL}
	vpermt2b	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{VBMI,VL} Disp8
	vpermt2b	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{VBMI,VL}
	vpmultishiftqb	xmm30, xmm29, xmm28	 # AVX512{VBMI,VL}
	vpmultishiftqb	xmm30{k7}, xmm29, xmm28	 # AVX512{VBMI,VL}
	vpmultishiftqb	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{VBMI,VL}
	vpmultishiftqb	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{VBMI,VL}
	vpmultishiftqb	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{VBMI,VL}
	vpmultishiftqb	xmm30, xmm29, [rcx]{1to2}	 # AVX512{VBMI,VL}
	vpmultishiftqb	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{VBMI,VL} Disp8
	vpmultishiftqb	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{VBMI,VL}
	vpmultishiftqb	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{VBMI,VL} Disp8
	vpmultishiftqb	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{VBMI,VL}
	vpmultishiftqb	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{VBMI,VL} Disp8
	vpmultishiftqb	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{VBMI,VL}
	vpmultishiftqb	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{VBMI,VL} Disp8
	vpmultishiftqb	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{VBMI,VL}
	vpmultishiftqb	ymm30, ymm29, ymm28	 # AVX512{VBMI,VL}
	vpmultishiftqb	ymm30{k7}, ymm29, ymm28	 # AVX512{VBMI,VL}
	vpmultishiftqb	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{VBMI,VL}
	vpmultishiftqb	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{VBMI,VL}
	vpmultishiftqb	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{VBMI,VL}
	vpmultishiftqb	ymm30, ymm29, [rcx]{1to4}	 # AVX512{VBMI,VL}
	vpmultishiftqb	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{VBMI,VL} Disp8
	vpmultishiftqb	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{VBMI,VL}
	vpmultishiftqb	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{VBMI,VL} Disp8
	vpmultishiftqb	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{VBMI,VL}
	vpmultishiftqb	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{VBMI,VL} Disp8
	vpmultishiftqb	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{VBMI,VL}
	vpmultishiftqb	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{VBMI,VL} Disp8
	vpmultishiftqb	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{VBMI,VL}
