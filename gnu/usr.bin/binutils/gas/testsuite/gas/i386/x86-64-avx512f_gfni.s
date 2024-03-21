# Check 64bit AVX512F,GFNI instructions

	.allow_index_reg
	.text
_start:
	vgf2p8affineqb	$0xab, %zmm28, %zmm29, %zmm30	 # AVX512F,GFNI
	vgf2p8affineqb	$0xab, %zmm28, %zmm29, %zmm30{%k7}	 # AVX512F,GFNI
	vgf2p8affineqb	$0xab, %zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F,GFNI
	vgf2p8affineqb	$123, 0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F,GFNI
	vgf2p8affineqb	$123, 8128(%rdx), %zmm29, %zmm30	 # AVX512F,GFNI Disp8
	vgf2p8affineqb	$123, 1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F,GFNI Disp8

	vgf2p8affineinvqb	$0xab, %zmm28, %zmm29, %zmm30	 # AVX512F,GFNI
	vgf2p8affineinvqb	$0xab, %zmm28, %zmm29, %zmm30{%k7}	 # AVX512F,GFNI
	vgf2p8affineinvqb	$0xab, %zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F,GFNI
	vgf2p8affineinvqb	$123, 0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F,GFNI
	vgf2p8affineinvqb	$123, 8128(%rdx), %zmm29, %zmm30	 # AVX512F,GFNI Disp8
	vgf2p8affineinvqb	$123, 1016(%rdx){1to8}, %zmm29, %zmm30	 # AVX512F,GFNI Disp8

	vgf2p8mulb	%zmm28, %zmm29, %zmm30	 # AVX512F,GFNI
	vgf2p8mulb	%zmm28, %zmm29, %zmm30{%k7}	 # AVX512F,GFNI
	vgf2p8mulb	%zmm28, %zmm29, %zmm30{%k7}{z}	 # AVX512F,GFNI
	vgf2p8mulb	0x123(%rax,%r14,8), %zmm29, %zmm30	 # AVX512F,GFNI
	vgf2p8mulb	8128(%rdx), %zmm29, %zmm30	 # AVX512F,GFNI Disp8

	.intel_syntax noprefix
	vgf2p8affineqb	zmm30, zmm29, zmm28, 0xab	 # AVX512F,GFNI
	vgf2p8affineqb	zmm30{k7}, zmm29, zmm28, 0xab	 # AVX512F,GFNI
	vgf2p8affineqb	zmm30{k7}{z}, zmm29, zmm28, 0xab	 # AVX512F,GFNI
	vgf2p8affineqb	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F,GFNI
	vgf2p8affineqb	zmm30, zmm29, ZMMWORD PTR [rdx+8128], 123	 # AVX512F,GFNI Disp8
	vgf2p8affineqb	zmm30, zmm29, [rdx+1016]{1to8}, 123	 # AVX512F,GFNI Disp8

	vgf2p8affineinvqb	zmm30, zmm29, zmm28, 0xab	 # AVX512F,GFNI
	vgf2p8affineinvqb	zmm30{k7}, zmm29, zmm28, 0xab	 # AVX512F,GFNI
	vgf2p8affineinvqb	zmm30{k7}{z}, zmm29, zmm28, 0xab	 # AVX512F,GFNI
	vgf2p8affineinvqb	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512F,GFNI
	vgf2p8affineinvqb	zmm30, zmm29, ZMMWORD PTR [rdx+8128], 123	 # AVX512F,GFNI Disp8
	vgf2p8affineinvqb	zmm30, zmm29, [rdx+1024]{1to8}, 123	 # AVX512F,GFNI

	vgf2p8mulb	zmm30, zmm29, zmm28	 # AVX512F,GFNI
	vgf2p8mulb	zmm30{k7}, zmm29, zmm28	 # AVX512F,GFNI
	vgf2p8mulb	zmm30{k7}{z}, zmm29, zmm28	 # AVX512F,GFNI
	vgf2p8mulb	zmm30, zmm29, ZMMWORD PTR [rax+r14*8+0x1234]	 # AVX512F,GFNI
	vgf2p8mulb	zmm30, zmm29, ZMMWORD PTR [rdx+8128]	 # AVX512F,GFNI Disp8
