# Check 32bit AVX512F,GFNI instructions

	.allow_index_reg
	.text
_start:
	vgf2p8affineqb	$0xab, %zmm4, %zmm5, %zmm6	 # AVX512F,GFNI
	vgf2p8affineqb	$0xab, %zmm4, %zmm5, %zmm6{%k7}	 # AVX512F,GFNI
	vgf2p8affineqb	$0xab, %zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F,GFNI
	vgf2p8affineqb	$123, -123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F,GFNI
	vgf2p8affineqb	$123, 8128(%edx), %zmm5, %zmm6	 # AVX512F,GFNI Disp8
	vgf2p8affineqb	$123, 1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F,GFNI Disp8

	vgf2p8affineinvqb	$0xab, %zmm4, %zmm5, %zmm6	 # AVX512F,GFNI
	vgf2p8affineinvqb	$0xab, %zmm4, %zmm5, %zmm6{%k7}	 # AVX512F,GFNI
	vgf2p8affineinvqb	$0xab, %zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F,GFNI
	vgf2p8affineinvqb	$123, -123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F,GFNI
	vgf2p8affineinvqb	$123, 8128(%edx), %zmm5, %zmm6	 # AVX512F,GFNI Disp8
	vgf2p8affineinvqb	$123, 1016(%edx){1to8}, %zmm5, %zmm6	 # AVX512F,GFNI Disp8

	vgf2p8mulb	%zmm4, %zmm5, %zmm6	 # AVX512F,GFNI
	vgf2p8mulb	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512F,GFNI
	vgf2p8mulb	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512F,GFNI
	vgf2p8mulb	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512F,GFNI
	vgf2p8mulb	8128(%edx), %zmm5, %zmm6	 # AVX512F,GFNI Disp8

	.intel_syntax noprefix
	vgf2p8affineqb	zmm6, zmm5, zmm4, 0xab	 # AVX512F,GFNI
	vgf2p8affineqb	zmm6{k7}, zmm5, zmm4, 0xab	 # AVX512F,GFNI
	vgf2p8affineqb	zmm6{k7}{z}, zmm5, zmm4, 0xab	 # AVX512F,GFNI
	vgf2p8affineqb	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F,GFNI
	vgf2p8affineqb	zmm6, zmm5, ZMMWORD PTR [edx+8128], 123	 # AVX512F,GFNI Disp8
	vgf2p8affineqb	zmm6, zmm5, [edx+1016]{1to8}, 123	 # AVX512F,GFNI Disp8

	vgf2p8affineinvqb	zmm6, zmm5, zmm4, 0xab	 # AVX512F,GFNI
	vgf2p8affineinvqb	zmm6{k7}, zmm5, zmm4, 0xab	 # AVX512F,GFNI
	vgf2p8affineinvqb	zmm6{k7}{z}, zmm5, zmm4, 0xab	 # AVX512F,GFNI
	vgf2p8affineinvqb	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512F,GFNI
	vgf2p8affineinvqb	zmm6, zmm5, ZMMWORD PTR [edx+8128], 123	 # AVX512F,GFNI Disp8
	vgf2p8affineinvqb	zmm6, zmm5, [edx+1016]{1to8}, 123	 # AVX512F,GFNI Disp8

	vgf2p8mulb	zmm6, zmm5, zmm4	 # AVX512F,GFNI
	vgf2p8mulb	zmm6{k7}, zmm5, zmm4	 # AVX512F,GFNI
	vgf2p8mulb	zmm6{k7}{z}, zmm5, zmm4	 # AVX512F,GFNI
	vgf2p8mulb	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512F,GFNI
	vgf2p8mulb	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512F,GFNI Disp8
