# Check AVX GFNI instructions

.allow_index_reg
.text
_start:
	vgf2p8mulb %ymm4, %ymm5, %ymm6
	vgf2p8mulb -123456(%rax,%r14,8), %ymm5, %ymm6
	vgf2p8mulb 126(%rdx), %ymm5, %ymm6

	vgf2p8affineqb $0xab, %ymm4, %ymm5, %ymm6
	vgf2p8affineqb $123, -123456(%rax,%r14,8), %ymm5, %ymm6
	vgf2p8affineqb $123, 126(%rdx), %ymm5, %ymm6

	vgf2p8affineinvqb $0xab, %ymm4, %ymm5, %ymm6
	vgf2p8affineinvqb $123, -123456(%rax,%r14,8), %ymm5, %ymm6
	vgf2p8affineinvqb $123, 126(%rdx), %ymm5, %ymm6

	vgf2p8mulb %xmm4, %xmm5, %xmm6
	vgf2p8mulb -123456(%rax,%r14,8), %xmm5, %xmm6
	vgf2p8mulb 126(%rdx), %xmm5, %xmm6

	vgf2p8affineqb $0xab, %xmm4, %xmm5, %xmm6
	vgf2p8affineqb $123, -123456(%rax,%r14,8), %xmm5, %xmm6
	vgf2p8affineqb $123, 126(%rdx), %xmm5, %xmm6

	vgf2p8affineinvqb $0xab, %xmm4, %xmm5, %xmm6
	vgf2p8affineinvqb $123, -123456(%rax,%r14,8), %xmm5, %xmm6
	vgf2p8affineinvqb $123, 126(%rdx), %xmm5, %xmm6

.intel_syntax noprefix

	vgf2p8mulb ymm6, ymm5, ymm4
	vgf2p8mulb ymm6, ymm5, YMMWORD PTR [rax+r14*8-123456]
	vgf2p8mulb ymm6, ymm5, YMMWORD PTR [rdx+126]

	vgf2p8affineqb ymm6, ymm5, ymm4, 0xab
	vgf2p8affineqb ymm6, ymm5, YMMWORD PTR [rax+r14*8-123456], 123
	vgf2p8affineqb ymm6, ymm5, YMMWORD PTR [rdx+126], 123

	vgf2p8affineinvqb ymm6, ymm5, ymm4, 0xab
	vgf2p8affineinvqb ymm6, ymm5, YMMWORD PTR [rax+r14*8-123456], 123
	vgf2p8affineinvqb ymm6, ymm5, YMMWORD PTR [rdx+126], 123

	vgf2p8mulb xmm6, xmm5, xmm4
	vgf2p8mulb xmm6, xmm5, XMMWORD PTR [rax+r14*8-123456]
	vgf2p8mulb xmm6, xmm5, XMMWORD PTR [rdx+126]

	vgf2p8affineqb xmm6, xmm5, xmm4, 0xab
	vgf2p8affineqb xmm6, xmm5, XMMWORD PTR [rax+r14*8-123456], 123
	vgf2p8affineqb xmm6, xmm5, XMMWORD PTR [rdx+126], 123

	vgf2p8affineinvqb xmm6, xmm5, xmm4, 0xab
	vgf2p8affineinvqb xmm6, xmm5, XMMWORD PTR [rax+r14*8-123456], 123
	vgf2p8affineinvqb xmm6, xmm5, XMMWORD PTR [rdx+126], 123
