# Check GFNI instructions

	.allow_index_reg
	.text
_start:
	gf2p8mulb %xmm4, %xmm5
	gf2p8mulb -123456(%esp,%esi,8), %xmm5
	gf2p8mulb 2032(%edx), %xmm5

	gf2p8affineqb $0xab, %xmm4, %xmm5
	gf2p8affineqb $123, -123456(%esp,%esi,8), %xmm5
	gf2p8affineqb $123, 2032(%edx), %xmm5

	gf2p8affineinvqb $0xab, %xmm4, %xmm5
	gf2p8affineinvqb $123, -123456(%esp,%esi,8), %xmm5
	gf2p8affineinvqb $123, 2032(%edx), %xmm5

	.intel_syntax noprefix

	gf2p8mulb xmm5, xmm4
	gf2p8mulb xmm5, XMMWORD PTR [esp+esi*8-123456]
	gf2p8mulb xmm5, XMMWORD PTR [edx+2032]

	gf2p8affineqb xmm5, xmm4, 0xab
	gf2p8affineqb xmm5, XMMWORD PTR [esp+esi*8-123456], 123
	gf2p8affineqb xmm5, XMMWORD PTR [edx+2032], 123

	gf2p8affineinvqb xmm5, xmm4, 0xab
	gf2p8affineinvqb xmm5, XMMWORD PTR [esp+esi*8-123456], 123
	gf2p8affineinvqb xmm5, XMMWORD PTR [edx+2032], 123
