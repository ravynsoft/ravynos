# Note: This file is also used by an ld test case.

	.text
	.global _start
_start:
	ret

	.data
	.p2align 4
l:	.long	0x11111111, 0x22222222
q:	.quad	0x4444444433333333

	.reloc l, BFD_RELOC_64, q
	.reloc q, BFD_RELOC_32, l
