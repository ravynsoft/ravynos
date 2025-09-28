# Check 64bit AVX512{BITALG,VL} instructions

	.allow_index_reg
	.text
_start:
	vpshufbitqmb	%xmm28, %xmm29, %k5	 # AVX512{BITALG,VL}
	vpshufbitqmb	%xmm28, %xmm29, %k5{%k7}	 # AVX512{BITALG,VL}
	vpshufbitqmb	0x123(%rax,%r14,8), %xmm29, %k5	 # AVX512{BITALG,VL}
	vpshufbitqmb	2032(%rdx), %xmm29, %k5	 # AVX512{BITALG,VL} Disp8
	vpshufbitqmb	%ymm28, %ymm29, %k5	 # AVX512{BITALG,VL}
	vpshufbitqmb	%ymm28, %ymm29, %k5{%k7}	 # AVX512{BITALG,VL}
	vpshufbitqmb	0x123(%rax,%r14,8), %ymm29, %k5	 # AVX512{BITALG,VL}
	vpshufbitqmb	4064(%rdx), %ymm29, %k5	 # AVX512{BITALG,VL} Disp8

	vpopcntb	%xmm29, %xmm30	 # AVX512{BITALG,VL}
	vpopcntb	%xmm29, %xmm30{%k7}	 # AVX512{BITALG,VL}
	vpopcntb	%xmm29, %xmm30{%k7}{z}	 # AVX512{BITALG,VL}
	vpopcntb	0x123(%rax,%r14,8), %xmm30	 # AVX512{BITALG,VL}
	vpopcntb	2032(%rdx), %xmm30	 # AVX512{BITALG,VL} Disp8
	vpopcntb	%ymm29, %ymm30	 # AVX512{BITALG,VL}
	vpopcntb	%ymm29, %ymm30{%k7}	 # AVX512{BITALG,VL}
	vpopcntb	%ymm29, %ymm30{%k7}{z}	 # AVX512{BITALG,VL}
	vpopcntb	0x123(%rax,%r14,8), %ymm30	 # AVX512{BITALG,VL}
	vpopcntb	4064(%rdx), %ymm30	 # AVX512{BITALG,VL} Disp8

	vpopcntw	%xmm29, %xmm30	 # AVX512{BITALG,VL}
	vpopcntw	%xmm29, %xmm30{%k7}	 # AVX512{BITALG,VL}
	vpopcntw	%xmm29, %xmm30{%k7}{z}	 # AVX512{BITALG,VL}
	vpopcntw	0x123(%rax,%r14,8), %xmm30	 # AVX512{BITALG,VL}
	vpopcntw	2032(%rdx), %xmm30	 # AVX512{BITALG,VL} Disp8
	vpopcntw	%ymm29, %ymm30	 # AVX512{BITALG,VL}
	vpopcntw	%ymm29, %ymm30{%k7}	 # AVX512{BITALG,VL}
	vpopcntw	%ymm29, %ymm30{%k7}{z}	 # AVX512{BITALG,VL}
	vpopcntw	0x123(%rax,%r14,8), %ymm30	 # AVX512{BITALG,VL}
	vpopcntw	4064(%rdx), %ymm30	 # AVX512{BITALG,VL} Disp8

	vpopcntd	%xmm29, %xmm30	 # AVX512{BITALG,VL}
	vpopcntd	%xmm29, %xmm30{%k7}	 # AVX512{BITALG,VL}
	vpopcntd	%xmm29, %xmm30{%k7}{z}	 # AVX512{BITALG,VL}
	vpopcntd	0x123(%rax,%r14,8), %xmm30	 # AVX512{BITALG,VL}
	vpopcntd	2032(%rdx), %xmm30	 # AVX512{BITALG,VL} Disp8
	vpopcntd	508(%rdx){1to4}, %xmm30	 # AVX512{BITALG,VL} Disp8
	vpopcntd	%ymm29, %ymm30	 # AVX512{BITALG,VL}
	vpopcntd	%ymm29, %ymm30{%k7}	 # AVX512{BITALG,VL}
	vpopcntd	%ymm29, %ymm30{%k7}{z}	 # AVX512{BITALG,VL}
	vpopcntd	0x123(%rax,%r14,8), %ymm30	 # AVX512{BITALG,VL}
	vpopcntd	4064(%rdx), %ymm30	 # AVX512{BITALG,VL} Disp8
	vpopcntd	508(%rdx){1to8}, %ymm30	 # AVX512{BITALG,VL} Disp8

	vpopcntq	%xmm29, %xmm30	 # AVX512{BITALG,VL}
	vpopcntq	%xmm29, %xmm30{%k7}	 # AVX512{BITALG,VL}
	vpopcntq	%xmm29, %xmm30{%k7}{z}	 # AVX512{BITALG,VL}
	vpopcntq	0x123(%rax,%r14,8), %xmm30	 # AVX512{BITALG,VL}
	vpopcntq	2032(%rdx), %xmm30	 # AVX512{BITALG,VL} Disp8
	vpopcntq	1016(%rdx){1to2}, %xmm30	 # AVX512{BITALG,VL} Disp8
	vpopcntq	%ymm29, %ymm30	 # AVX512{BITALG,VL}
	vpopcntq	%ymm29, %ymm30{%k7}	 # AVX512{BITALG,VL}
	vpopcntq	%ymm29, %ymm30{%k7}{z}	 # AVX512{BITALG,VL}
	vpopcntq	0x123(%rax,%r14,8), %ymm30	 # AVX512{BITALG,VL}
	vpopcntq	4064(%rdx), %ymm30	 # AVX512{BITALG,VL} Disp8
	vpopcntq	1016(%rdx){1to4}, %ymm30	 # AVX512{BITALG,VL} Disp8

	.intel_syntax noprefix
	vpshufbitqmb	k5, xmm29, xmm28	 # AVX512{BITALG,VL}
	vpshufbitqmb	k5{k7}, xmm29, xmm28	 # AVX512{BITALG,VL}
	vpshufbitqmb	k5, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BITALG,VL}
	vpshufbitqmb	k5, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BITALG,VL} Disp8
	vpshufbitqmb	k5, ymm29, ymm28	 # AVX512{BITALG,VL}
	vpshufbitqmb	k5{k7}, ymm29, ymm28	 # AVX512{BITALG,VL}
	vpshufbitqmb	k5, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BITALG,VL}
	vpshufbitqmb	k5, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BITALG,VL} Disp8

	vpopcntb	xmm30, xmm29	 # AVX512{BITALG,VL}
	vpopcntb	xmm30{k7}, xmm29	 # AVX512{BITALG,VL}
	vpopcntb	xmm30{k7}{z}, xmm29	 # AVX512{BITALG,VL}
	vpopcntb	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BITALG,VL}
	vpopcntb	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{BITALG,VL} Disp8
	vpopcntb	ymm30, ymm29	 # AVX512{BITALG,VL}
	vpopcntb	ymm30{k7}, ymm29	 # AVX512{BITALG,VL}
	vpopcntb	ymm30{k7}{z}, ymm29	 # AVX512{BITALG,VL}
	vpopcntb	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BITALG,VL}
	vpopcntb	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{BITALG,VL} Disp8

	vpopcntw	xmm30, xmm29	 # AVX512{BITALG,VL}
	vpopcntw	xmm30{k7}, xmm29	 # AVX512{BITALG,VL}
	vpopcntw	xmm30{k7}{z}, xmm29	 # AVX512{BITALG,VL}
	vpopcntw	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BITALG,VL}
	vpopcntw	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{BITALG,VL} Disp8
	vpopcntw	ymm30, ymm29	 # AVX512{BITALG,VL}
	vpopcntw	ymm30{k7}, ymm29	 # AVX512{BITALG,VL}
	vpopcntw	ymm30{k7}{z}, ymm29	 # AVX512{BITALG,VL}
	vpopcntw	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BITALG,VL}
	vpopcntw	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{BITALG,VL} Disp8

	vpopcntd	xmm30, xmm29	 # AVX512{BITALG,VL}
	vpopcntd	xmm30{k7}, xmm29	 # AVX512{BITALG,VL}
	vpopcntd	xmm30{k7}{z}, xmm29	 # AVX512{BITALG,VL}
	vpopcntd	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BITALG,VL}
	vpopcntd	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{BITALG,VL} Disp8
	vpopcntd	xmm30, [rdx+508]{1to4}	 # AVX512{BITALG,VL} Disp8
	vpopcntd	ymm30, ymm29	 # AVX512{BITALG,VL}
	vpopcntd	ymm30{k7}, ymm29	 # AVX512{BITALG,VL}
	vpopcntd	ymm30{k7}{z}, ymm29	 # AVX512{BITALG,VL}
	vpopcntd	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BITALG,VL}
	vpopcntd	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{BITALG,VL} Disp8
	vpopcntd	ymm30, [rdx+508]{1to8}	 # AVX512{BITALG,VL} Disp8

	vpopcntq	xmm30, xmm29	 # AVX512{BITALG,VL}
	vpopcntq	xmm30{k7}, xmm29	 # AVX512{BITALG,VL}
	vpopcntq	xmm30{k7}{z}, xmm29	 # AVX512{BITALG,VL}
	vpopcntq	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BITALG,VL}
	vpopcntq	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{BITALG,VL} Disp8
	vpopcntq	xmm30, [rdx+1016]{1to2}	 # AVX512{BITALG,VL} Disp8
	vpopcntq	ymm30, ymm29	 # AVX512{BITALG,VL}
	vpopcntq	ymm30{k7}, ymm29	 # AVX512{BITALG,VL}
	vpopcntq	ymm30{k7}{z}, ymm29	 # AVX512{BITALG,VL}
	vpopcntq	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BITALG,VL}
	vpopcntq	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{BITALG,VL} Disp8
	vpopcntq	ymm30, [rdx+1016]{1to4}	 # AVX512{BITALG,VL} Disp8
