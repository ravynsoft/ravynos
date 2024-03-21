# Check 64bit AVX512VL,GFNI instructions

	.allow_index_reg
	.text
_start:
	vgf2p8affineqb	$0xab, %xmm28, %xmm29, %xmm30	 # AVX512VL,GFNI
	vgf2p8affineqb	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512VL,GFNI
	vgf2p8affineqb	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512VL,GFNI
	vgf2p8affineqb	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512VL,GFNI
	vgf2p8affineqb	$123, 2032(%rdx), %xmm29, %xmm30	 # AVX512VL,GFNI Disp8
	vgf2p8affineqb	$123, 1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512VL,GFNI Disp8
	vgf2p8affineqb	$0xab, %ymm28, %ymm29, %ymm30	 # AVX512VL,GFNI
	vgf2p8affineqb	$0xab, %ymm28, %ymm29, %ymm30{%k7}	 # AVX512VL,GFNI
	vgf2p8affineqb	$0xab, %ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512VL,GFNI
	vgf2p8affineqb	$123, 0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512VL,GFNI
	vgf2p8affineqb	$123, 4064(%rdx), %ymm29, %ymm30	 # AVX512VL,GFNI Disp8
	vgf2p8affineqb	$123, 1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512VL,GFNI Disp8

	vgf2p8affineinvqb	$0xab, %xmm28, %xmm29, %xmm30	 # AVX512VL,GFNI
	vgf2p8affineinvqb	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512VL,GFNI
	vgf2p8affineinvqb	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512VL,GFNI
	vgf2p8affineinvqb	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512VL,GFNI
	vgf2p8affineinvqb	$123, 2032(%rdx), %xmm29, %xmm30	 # AVX512VL,GFNI Disp8
	vgf2p8affineinvqb	$123, 1016(%rdx){1to2}, %xmm29, %xmm30	 # AVX512VL,GFNI Disp8
	vgf2p8affineinvqb	$0xab, %ymm28, %ymm29, %ymm30	 # AVX512VL,GFNI
	vgf2p8affineinvqb	$0xab, %ymm28, %ymm29, %ymm30{%k7}	 # AVX512VL,GFNI
	vgf2p8affineinvqb	$0xab, %ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512VL,GFNI
	vgf2p8affineinvqb	$123, 0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512VL,GFNI
	vgf2p8affineinvqb	$123, 4064(%rdx), %ymm29, %ymm30	 # AVX512VL,GFNI Disp8
	vgf2p8affineinvqb	$123, 1016(%rdx){1to4}, %ymm29, %ymm30	 # AVX512VL,GFNI Disp8

	vgf2p8mulb	%xmm28, %xmm29, %xmm30	 # AVX512VL,GFNI
	vgf2p8mulb	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512VL,GFNI
	vgf2p8mulb	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512VL,GFNI
	vgf2p8mulb	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512VL,GFNI
	vgf2p8mulb	2032(%rdx), %xmm29, %xmm30	 # AVX512VL,GFNI Disp8
	vgf2p8mulb	%ymm28, %ymm29, %ymm30	 # AVX512VL,GFNI
	vgf2p8mulb	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512VL,GFNI
	vgf2p8mulb	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512VL,GFNI
	vgf2p8mulb	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512VL,GFNI
	vgf2p8mulb	4064(%rdx), %ymm29, %ymm30	 # AVX512VL,GFNI Disp8

	.intel_syntax noprefix
	vgf2p8affineqb	xmm30, xmm29, xmm28, 0xab	 # AVX512VL,GFNI
	vgf2p8affineqb	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512VL,GFNI
	vgf2p8affineqb	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512VL,GFNI
	vgf2p8affineqb	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512VL,GFNI
	vgf2p8affineqb	xmm30, xmm29, XMMWORD PTR [rdx+2032], 123	 # AVX512VL,GFNI Disp8
	vgf2p8affineqb	xmm30, xmm29, [rdx+1016]{1to2}, 123	 # AVX512VL,GFNI Disp8
	vgf2p8affineqb	ymm30, ymm29, ymm28, 0xab	 # AVX512VL,GFNI
	vgf2p8affineqb	ymm30{k7}, ymm29, ymm28, 0xab	 # AVX512VL,GFNI
	vgf2p8affineqb	ymm30{k7}{z}, ymm29, ymm28, 0xab	 # AVX512VL,GFNI
	vgf2p8affineqb	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512VL,GFNI
	vgf2p8affineqb	ymm30, ymm29, YMMWORD PTR [rdx+4064], 123	 # AVX512VL,GFNI Disp8
	vgf2p8affineqb	ymm30, ymm29, [rdx+1016]{1to4}, 123	 # AVX512VL,GFNI Disp8

	vgf2p8affineinvqb	xmm30, xmm29, xmm28, 0xab	 # AVX512VL,GFNI
	vgf2p8affineinvqb	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512VL,GFNI
	vgf2p8affineinvqb	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512VL,GFNI
	vgf2p8affineinvqb	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512VL,GFNI
	vgf2p8affineinvqb	xmm30, xmm29, XMMWORD PTR [rdx+2032], 123	 # AVX512VL,GFNI Disp8
	vgf2p8affineinvqb	xmm30, xmm29, [rdx+1016]{1to2}, 123	 # AVX512VL,GFNI Disp8
	vgf2p8affineinvqb	ymm30, ymm29, ymm28, 0xab	 # AVX512VL,GFNI
	vgf2p8affineinvqb	ymm30{k7}, ymm29, ymm28, 0xab	 # AVX512VL,GFNI
	vgf2p8affineinvqb	ymm30{k7}{z}, ymm29, ymm28, 0xab	 # AVX512VL,GFNI
	vgf2p8affineinvqb	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512VL,GFNI
	vgf2p8affineinvqb	ymm30, ymm29, YMMWORD PTR [rdx+4064], 123	 # AVX512VL,GFNI Disp8
	vgf2p8affineinvqb	ymm30, ymm29, [rdx+1024]{1to4}, 123	 # AVX512VL,GFNI

	vgf2p8mulb	xmm30, xmm29, xmm28	 # AVX512VL,GFNI
	vgf2p8mulb	xmm30{k7}, xmm29, xmm28	 # AVX512VL,GFNI
	vgf2p8mulb	xmm30{k7}{z}, xmm29, xmm28	 # AVX512VL,GFNI
	vgf2p8mulb	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512VL,GFNI
	vgf2p8mulb	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512VL,GFNI Disp8
	vgf2p8mulb	ymm30, ymm29, ymm28	 # AVX512VL,GFNI
	vgf2p8mulb	ymm30{k7}, ymm29, ymm28	 # AVX512VL,GFNI
	vgf2p8mulb	ymm30{k7}{z}, ymm29, ymm28	 # AVX512VL,GFNI
	vgf2p8mulb	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512VL,GFNI
	vgf2p8mulb	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512VL,GFNI Disp8
