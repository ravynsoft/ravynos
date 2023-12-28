# Check 32bit AVX512VL,GFNI instructions

	.allow_index_reg
	.text
_start:
	vgf2p8affineqb	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512VL,GFNI
	vgf2p8affineqb	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512VL,GFNI
	vgf2p8affineqb	$123, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512VL,GFNI
	vgf2p8affineqb	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512VL,GFNI
	vgf2p8affineqb	$123, 2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512VL,GFNI Disp8
	vgf2p8affineqb	$123, 1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512VL,GFNI Disp8
	vgf2p8affineqb	$0xab, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512VL,GFNI
	vgf2p8affineqb	$0xab, %ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512VL,GFNI
	vgf2p8affineqb	$123, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512VL,GFNI
	vgf2p8affineqb	$123, -123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512VL,GFNI
	vgf2p8affineqb	$123, 4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512VL,GFNI Disp8
	vgf2p8affineqb	$123, 1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512VL,GFNI Disp8

	vgf2p8affineinvqb	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512VL,GFNI
	vgf2p8affineinvqb	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512VL,GFNI
	vgf2p8affineinvqb	$123, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512VL,GFNI
	vgf2p8affineinvqb	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512VL,GFNI
	vgf2p8affineinvqb	$123, 2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512VL,GFNI Disp8
	vgf2p8affineinvqb	$123, 1016(%edx){1to2}, %xmm5, %xmm6{%k7}	 # AVX512VL,GFNI Disp8
	vgf2p8affineinvqb	$0xab, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512VL,GFNI
	vgf2p8affineinvqb	$0xab, %ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512VL,GFNI
	vgf2p8affineinvqb	$123, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512VL,GFNI
	vgf2p8affineinvqb	$123, -123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512VL,GFNI
	vgf2p8affineinvqb	$123, 4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512VL,GFNI Disp8
	vgf2p8affineinvqb	$123, 1016(%edx){1to4}, %ymm5, %ymm6{%k7}	 # AVX512VL,GFNI Disp8

	vgf2p8mulb	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512VL,GFNI
	vgf2p8mulb	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512VL,GFNI
	vgf2p8mulb	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512VL,GFNI
	vgf2p8mulb	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512VL,GFNI Disp8
	vgf2p8mulb	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512VL,GFNI
	vgf2p8mulb	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512VL,GFNI
	vgf2p8mulb	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512VL,GFNI
	vgf2p8mulb	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512VL,GFNI Disp8

	.intel_syntax noprefix
	vgf2p8affineqb	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512VL,GFNI
	vgf2p8affineqb	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512VL,GFNI
	vgf2p8affineqb	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512VL,GFNI
	vgf2p8affineqb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032], 123	 # AVX512VL,GFNI Disp8
	vgf2p8affineqb	xmm6{k7}, xmm5, [edx+1016]{1to2}, 123	 # AVX512VL,GFNI Disp8
	vgf2p8affineqb	ymm6{k7}, ymm5, ymm4, 0xab	 # AVX512VL,GFNI
	vgf2p8affineqb	ymm6{k7}{z}, ymm5, ymm4, 0xab	 # AVX512VL,GFNI
	vgf2p8affineqb	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512VL,GFNI
	vgf2p8affineqb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064], 123	 # AVX512VL,GFNI Disp8
	vgf2p8affineqb	ymm6{k7}, ymm5, [edx+1016]{1to4}, 123	 # AVX512VL,GFNI Disp8

	vgf2p8affineinvqb	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512VL,GFNI
	vgf2p8affineinvqb	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512VL,GFNI
	vgf2p8affineinvqb	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512VL,GFNI
	vgf2p8affineinvqb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032], 123	 # AVX512VL,GFNI Disp8
	vgf2p8affineinvqb	xmm6{k7}, xmm5, [edx+1016]{1to2}, 123	 # AVX512VL,GFNI Disp8
	vgf2p8affineinvqb	ymm6{k7}, ymm5, ymm4, 0xab	 # AVX512VL,GFNI
	vgf2p8affineinvqb	ymm6{k7}{z}, ymm5, ymm4, 0xab	 # AVX512VL,GFNI
	vgf2p8affineinvqb	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512VL,GFNI
	vgf2p8affineinvqb	ymm6{k7}, ymm5, [eax]{1to4}, 123	 # AVX512VL,GFNI
	vgf2p8affineinvqb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064], 123	 # AVX512VL,GFNI Disp8
	vgf2p8affineinvqb	ymm6{k7}, ymm5, [edx+1016]{1to4}, 123	 # AVX512VL,GFNI Disp8

	vgf2p8mulb	xmm6{k7}, xmm5, xmm4	 # AVX512VL,GFNI
	vgf2p8mulb	xmm6{k7}{z}, xmm5, xmm4	 # AVX512VL,GFNI
	vgf2p8mulb	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512VL,GFNI
	vgf2p8mulb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512VL,GFNI Disp8
	vgf2p8mulb	ymm6{k7}, ymm5, ymm4	 # AVX512VL,GFNI
	vgf2p8mulb	ymm6{k7}{z}, ymm5, ymm4	 # AVX512VL,GFNI
	vgf2p8mulb	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512VL,GFNI
	vgf2p8mulb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512VL,GFNI Disp8
