	.text
	.arch corei7
	.arch .avx512f
_start:
	vmovntdq	%zmm20, (%rcx)
