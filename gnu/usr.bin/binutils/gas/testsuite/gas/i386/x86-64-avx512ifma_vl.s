# Check 64bit AVX512{IFMA,VL} instructions

	.allow_index_reg
	.text
_start:
	vpmadd52luq	%xmm28, %xmm29, %xmm30	 # AVX512{IFMA,VL}
	vpmadd52luq	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{IFMA,VL}
	vpmadd52luq	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{IFMA,VL}
	vpmadd52luq	(%rcx), %xmm29, %xmm30	 # AVX512{IFMA,VL}
	vpmadd52luq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{IFMA,VL}
	vpmadd52luq	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{IFMA,VL}
	vpmadd52luq	2032(%rdx), %xmm29, %xmm30	 # AVX512{IFMA,VL} Disp8
	vpmadd52luq	2048(%rdx), %xmm29, %xmm30	 # AVX512{IFMA,VL}
	vpmadd52luq	-2048(%rdx), %xmm29, %xmm30	 # AVX512{IFMA,VL} Disp8
	vpmadd52luq	-2064(%rdx), %xmm29, %xmm30	 # AVX512{IFMA,VL}
	vpmadd52luq	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{IFMA,VL} Disp8
	vpmadd52luq	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{IFMA,VL}
	vpmadd52luq	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{IFMA,VL} Disp8
	vpmadd52luq	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{IFMA,VL}
	vpmadd52luq	%ymm28, %ymm29, %ymm30	 # AVX512{IFMA,VL}
	vpmadd52luq	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{IFMA,VL}
	vpmadd52luq	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{IFMA,VL}
	vpmadd52luq	(%rcx), %ymm29, %ymm30	 # AVX512{IFMA,VL}
	vpmadd52luq	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{IFMA,VL}
	vpmadd52luq	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{IFMA,VL}
	vpmadd52luq	4064(%rdx), %ymm29, %ymm30	 # AVX512{IFMA,VL} Disp8
	vpmadd52luq	4096(%rdx), %ymm29, %ymm30	 # AVX512{IFMA,VL}
	vpmadd52luq	-4096(%rdx), %ymm29, %ymm30	 # AVX512{IFMA,VL} Disp8
	vpmadd52luq	-4128(%rdx), %ymm29, %ymm30	 # AVX512{IFMA,VL}
	vpmadd52luq	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{IFMA,VL} Disp8
	vpmadd52luq	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{IFMA,VL}
	vpmadd52luq	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{IFMA,VL} Disp8
	vpmadd52luq	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{IFMA,VL}
	vpmadd52huq	%xmm28, %xmm29, %xmm30	 # AVX512{IFMA,VL}
	vpmadd52huq	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{IFMA,VL}
	vpmadd52huq	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{IFMA,VL}
	vpmadd52huq	(%rcx), %xmm29, %xmm30	 # AVX512{IFMA,VL}
	vpmadd52huq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{IFMA,VL}
	vpmadd52huq	(%rcx){1to2}, %xmm29, %xmm30	 # AVX512{IFMA,VL}
	vpmadd52huq	2032(%rdx), %xmm29, %xmm30	 # AVX512{IFMA,VL} Disp8
	vpmadd52huq	2048(%rdx), %xmm29, %xmm30	 # AVX512{IFMA,VL}
	vpmadd52huq	-2048(%rdx), %xmm29, %xmm30	 # AVX512{IFMA,VL} Disp8
	vpmadd52huq	-2064(%rdx), %xmm29, %xmm30	 # AVX512{IFMA,VL}
	vpmadd52huq	1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{IFMA,VL} Disp8
	vpmadd52huq	1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{IFMA,VL}
	vpmadd52huq	-1024(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{IFMA,VL} Disp8
	vpmadd52huq	-1032(%rdx){1to2}, %xmm29, %xmm30	 # AVX512{IFMA,VL}
	vpmadd52huq	%ymm28, %ymm29, %ymm30	 # AVX512{IFMA,VL}
	vpmadd52huq	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{IFMA,VL}
	vpmadd52huq	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{IFMA,VL}
	vpmadd52huq	(%rcx), %ymm29, %ymm30	 # AVX512{IFMA,VL}
	vpmadd52huq	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{IFMA,VL}
	vpmadd52huq	(%rcx){1to4}, %ymm29, %ymm30	 # AVX512{IFMA,VL}
	vpmadd52huq	4064(%rdx), %ymm29, %ymm30	 # AVX512{IFMA,VL} Disp8
	vpmadd52huq	4096(%rdx), %ymm29, %ymm30	 # AVX512{IFMA,VL}
	vpmadd52huq	-4096(%rdx), %ymm29, %ymm30	 # AVX512{IFMA,VL} Disp8
	vpmadd52huq	-4128(%rdx), %ymm29, %ymm30	 # AVX512{IFMA,VL}
	vpmadd52huq	1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{IFMA,VL} Disp8
	vpmadd52huq	1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{IFMA,VL}
	vpmadd52huq	-1024(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{IFMA,VL} Disp8
	vpmadd52huq	-1032(%rdx){1to4}, %ymm29, %ymm30	 # AVX512{IFMA,VL}

	.intel_syntax noprefix
	vpmadd52luq	xmm30, xmm29, xmm28	 # AVX512{IFMA,VL}
	vpmadd52luq	xmm30{k7}, xmm29, xmm28	 # AVX512{IFMA,VL}
	vpmadd52luq	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{IFMA,VL}
	vpmadd52luq	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{IFMA,VL}
	vpmadd52luq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{IFMA,VL}
	vpmadd52luq	xmm30, xmm29, [rcx]{1to2}	 # AVX512{IFMA,VL}
	vpmadd52luq	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{IFMA,VL} Disp8
	vpmadd52luq	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{IFMA,VL}
	vpmadd52luq	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{IFMA,VL} Disp8
	vpmadd52luq	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{IFMA,VL}
	vpmadd52luq	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{IFMA,VL} Disp8
	vpmadd52luq	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{IFMA,VL}
	vpmadd52luq	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{IFMA,VL} Disp8
	vpmadd52luq	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{IFMA,VL}
	vpmadd52luq	ymm30, ymm29, ymm28	 # AVX512{IFMA,VL}
	vpmadd52luq	ymm30{k7}, ymm29, ymm28	 # AVX512{IFMA,VL}
	vpmadd52luq	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{IFMA,VL}
	vpmadd52luq	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{IFMA,VL}
	vpmadd52luq	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{IFMA,VL}
	vpmadd52luq	ymm30, ymm29, [rcx]{1to4}	 # AVX512{IFMA,VL}
	vpmadd52luq	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{IFMA,VL} Disp8
	vpmadd52luq	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{IFMA,VL}
	vpmadd52luq	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{IFMA,VL} Disp8
	vpmadd52luq	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{IFMA,VL}
	vpmadd52luq	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{IFMA,VL} Disp8
	vpmadd52luq	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{IFMA,VL}
	vpmadd52luq	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{IFMA,VL} Disp8
	vpmadd52luq	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{IFMA,VL}
	vpmadd52huq	xmm30, xmm29, xmm28	 # AVX512{IFMA,VL}
	vpmadd52huq	xmm30{k7}, xmm29, xmm28	 # AVX512{IFMA,VL}
	vpmadd52huq	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{IFMA,VL}
	vpmadd52huq	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{IFMA,VL}
	vpmadd52huq	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{IFMA,VL}
	vpmadd52huq	xmm30, xmm29, [rcx]{1to2}	 # AVX512{IFMA,VL}
	vpmadd52huq	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{IFMA,VL} Disp8
	vpmadd52huq	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{IFMA,VL}
	vpmadd52huq	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{IFMA,VL} Disp8
	vpmadd52huq	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{IFMA,VL}
	vpmadd52huq	xmm30, xmm29, [rdx+1016]{1to2}	 # AVX512{IFMA,VL} Disp8
	vpmadd52huq	xmm30, xmm29, [rdx+1024]{1to2}	 # AVX512{IFMA,VL}
	vpmadd52huq	xmm30, xmm29, [rdx-1024]{1to2}	 # AVX512{IFMA,VL} Disp8
	vpmadd52huq	xmm30, xmm29, [rdx-1032]{1to2}	 # AVX512{IFMA,VL}
	vpmadd52huq	ymm30, ymm29, ymm28	 # AVX512{IFMA,VL}
	vpmadd52huq	ymm30{k7}, ymm29, ymm28	 # AVX512{IFMA,VL}
	vpmadd52huq	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{IFMA,VL}
	vpmadd52huq	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{IFMA,VL}
	vpmadd52huq	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{IFMA,VL}
	vpmadd52huq	ymm30, ymm29, [rcx]{1to4}	 # AVX512{IFMA,VL}
	vpmadd52huq	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{IFMA,VL} Disp8
	vpmadd52huq	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{IFMA,VL}
	vpmadd52huq	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{IFMA,VL} Disp8
	vpmadd52huq	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{IFMA,VL}
	vpmadd52huq	ymm30, ymm29, [rdx+1016]{1to4}	 # AVX512{IFMA,VL} Disp8
	vpmadd52huq	ymm30, ymm29, [rdx+1024]{1to4}	 # AVX512{IFMA,VL}
	vpmadd52huq	ymm30, ymm29, [rdx-1024]{1to4}	 # AVX512{IFMA,VL} Disp8
	vpmadd52huq	ymm30, ymm29, [rdx-1032]{1to4}	 # AVX512{IFMA,VL}
