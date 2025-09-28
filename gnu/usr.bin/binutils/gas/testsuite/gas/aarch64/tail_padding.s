/* tail_padding.s Test file for AArch64 tail padding.  */

	.data
	.p2align 6
f:
	.space 8

	.bss
	.p2align 6
b:
	.space 6
	.local c
	.comm c, 6
