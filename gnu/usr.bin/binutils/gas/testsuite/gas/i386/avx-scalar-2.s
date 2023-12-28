# Check VEX.128 scalar instructions with -mavxscalar=256 -msse2avx

	.allow_index_reg
	.text
_start:

	movd %xmm4,(%ecx)
	movd %xmm4,%ecx
	movd (%ecx),%xmm4
	movd %ecx,%xmm4

	vmovd %xmm4,(%ecx)
	vmovd %xmm4,%ecx
	vmovd (%ecx),%xmm4
	vmovd %ecx,%xmm4

	movq %xmm4,(%ecx)
	movq (%ecx),%xmm4
	vmovq %xmm4,(%ecx)
	vmovq (%ecx),%xmm4

	vmovq %xmm4,%xmm6
