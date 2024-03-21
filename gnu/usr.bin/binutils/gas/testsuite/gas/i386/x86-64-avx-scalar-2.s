# Check VEX.128 scalar instructions with -mavxscalar=256 -msse2avx

	.allow_index_reg
	.text
_start:
	movd %xmm4,(%rcx)
	movd %xmm4,%ecx
	movd (%rcx),%xmm4
	movd %ecx,%xmm4

	movd %rcx,%xmm4
	movd %xmm4,%rcx

	movq %xmm4,(%rcx)
	movq %xmm4,%rcx
	movq (%rcx),%xmm4
	movq %rcx,%xmm4

	vmovd %xmm4,(%rcx)
	vmovd %xmm4,%ecx
	vmovd (%rcx),%xmm4
	vmovd %ecx,%xmm4

	vmovd %xmm4,%rcx
	vmovd %rcx,%xmm4

	vmovq %xmm4,(%rcx)
	vmovq %xmm4,%rcx
	vmovq (%rcx),%xmm4
	vmovq %rcx,%xmm4
	vmovq %xmm4,%xmm6
