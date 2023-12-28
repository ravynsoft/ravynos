# Check 32bit AVX512F instructions

	.allow_index_reg
	.text
_start:

	vmovapd.s	%zmm5, %zmm6	 # AVX512F
	vmovapd	%zmm5, %zmm6	 # AVX512F
	vmovapd.s	%zmm5, %zmm6{%k7}	 # AVX512F
	vmovapd	%zmm5, %zmm6{%k7}	 # AVX512F
	vmovapd.s	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vmovapd	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vmovaps.s	%zmm5, %zmm6	 # AVX512F
	vmovaps	%zmm5, %zmm6	 # AVX512F
	vmovaps.s	%zmm5, %zmm6{%k7}	 # AVX512F
	vmovaps	%zmm5, %zmm6{%k7}	 # AVX512F
	vmovaps.s	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vmovaps	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vmovdqa32.s	%zmm5, %zmm6	 # AVX512F
	vmovdqa32	%zmm5, %zmm6	 # AVX512F
	vmovdqa32.s	%zmm5, %zmm6{%k7}	 # AVX512F
	vmovdqa32	%zmm5, %zmm6{%k7}	 # AVX512F
	vmovdqa32.s	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vmovdqa32	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vmovdqa64.s	%zmm5, %zmm6	 # AVX512F
	vmovdqa64	%zmm5, %zmm6	 # AVX512F
	vmovdqa64.s	%zmm5, %zmm6{%k7}	 # AVX512F
	vmovdqa64	%zmm5, %zmm6{%k7}	 # AVX512F
	vmovdqa64.s	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vmovdqa64	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vmovdqu32.s	%zmm5, %zmm6	 # AVX512F
	vmovdqu32	%zmm5, %zmm6	 # AVX512F
	vmovdqu32.s	%zmm5, %zmm6{%k7}	 # AVX512F
	vmovdqu32	%zmm5, %zmm6{%k7}	 # AVX512F
	vmovdqu32.s	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vmovdqu32	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vmovdqu64.s	%zmm5, %zmm6	 # AVX512F
	vmovdqu64	%zmm5, %zmm6	 # AVX512F
	vmovdqu64.s	%zmm5, %zmm6{%k7}	 # AVX512F
	vmovdqu64	%zmm5, %zmm6{%k7}	 # AVX512F
	vmovdqu64.s	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vmovdqu64	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vmovsd.s	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vmovsd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vmovsd.s	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vmovsd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vmovss.s	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vmovss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512F
	vmovss.s	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vmovss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512F
	vmovupd.s	%zmm5, %zmm6	 # AVX512F
	vmovupd	%zmm5, %zmm6	 # AVX512F
	vmovupd.s	%zmm5, %zmm6{%k7}	 # AVX512F
	vmovupd	%zmm5, %zmm6{%k7}	 # AVX512F
	vmovupd.s	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vmovupd	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vmovups.s	%zmm5, %zmm6	 # AVX512F
	vmovups	%zmm5, %zmm6	 # AVX512F
	vmovups.s	%zmm5, %zmm6{%k7}	 # AVX512F
	vmovups	%zmm5, %zmm6{%k7}	 # AVX512F
	vmovups.s	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	vmovups	%zmm5, %zmm6{%k7}{z}	 # AVX512F
	{evex} vmovq.s	%xmm5,%xmm6
	{evex} vmovq	%xmm5,%xmm6

	.intel_syntax noprefix
	vmovapd.s	zmm6, zmm5	 # AVX512F
	vmovapd	zmm6, zmm5	 # AVX512F
	vmovapd.s	zmm6{k7}, zmm5	 # AVX512F
	vmovapd	zmm6{k7}, zmm5	 # AVX512F
	vmovapd.s	zmm6{k7}{z}, zmm5	 # AVX512F
	vmovapd	zmm6{k7}{z}, zmm5	 # AVX512F
	vmovaps.s	zmm6, zmm5	 # AVX512F
	vmovaps	zmm6, zmm5	 # AVX512F
	vmovaps.s	zmm6{k7}, zmm5	 # AVX512F
	vmovaps	zmm6{k7}, zmm5	 # AVX512F
	vmovaps.s	zmm6{k7}{z}, zmm5	 # AVX512F
	vmovaps	zmm6{k7}{z}, zmm5	 # AVX512F
	vmovdqa32.s	zmm6, zmm5	 # AVX512F
	vmovdqa32	zmm6, zmm5	 # AVX512F
	vmovdqa32.s	zmm6{k7}, zmm5	 # AVX512F
	vmovdqa32	zmm6{k7}, zmm5	 # AVX512F
	vmovdqa32.s	zmm6{k7}{z}, zmm5	 # AVX512F
	vmovdqa32	zmm6{k7}{z}, zmm5	 # AVX512F
	vmovdqa64.s	zmm6, zmm5	 # AVX512F
	vmovdqa64	zmm6, zmm5	 # AVX512F
	vmovdqa64.s	zmm6{k7}, zmm5	 # AVX512F
	vmovdqa64	zmm6{k7}, zmm5	 # AVX512F
	vmovdqa64.s	zmm6{k7}{z}, zmm5	 # AVX512F
	vmovdqa64	zmm6{k7}{z}, zmm5	 # AVX512F
	vmovdqu32.s	zmm6, zmm5	 # AVX512F
	vmovdqu32	zmm6, zmm5	 # AVX512F
	vmovdqu32.s	zmm6{k7}, zmm5	 # AVX512F
	vmovdqu32	zmm6{k7}, zmm5	 # AVX512F
	vmovdqu32.s	zmm6{k7}{z}, zmm5	 # AVX512F
	vmovdqu32	zmm6{k7}{z}, zmm5	 # AVX512F
	vmovdqu64.s	zmm6, zmm5	 # AVX512F
	vmovdqu64	zmm6, zmm5	 # AVX512F
	vmovdqu64.s	zmm6{k7}, zmm5	 # AVX512F
	vmovdqu64	zmm6{k7}, zmm5	 # AVX512F
	vmovdqu64.s	zmm6{k7}{z}, zmm5	 # AVX512F
	vmovdqu64	zmm6{k7}{z}, zmm5	 # AVX512F
	vmovsd.s	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vmovsd	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vmovsd.s	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vmovsd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vmovss.s	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vmovss	xmm6{k7}, xmm5, xmm4	 # AVX512F
	vmovss.s	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vmovss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512F
	vmovupd.s	zmm6, zmm5	 # AVX512F
	vmovupd	zmm6, zmm5	 # AVX512F
	vmovupd.s	zmm6{k7}, zmm5	 # AVX512F
	vmovupd	zmm6{k7}, zmm5	 # AVX512F
	vmovupd.s	zmm6{k7}{z}, zmm5	 # AVX512F
	vmovupd	zmm6{k7}{z}, zmm5	 # AVX512F
	vmovups.s	zmm6, zmm5	 # AVX512F
	vmovups	zmm6, zmm5	 # AVX512F
	vmovups.s	zmm6{k7}, zmm5	 # AVX512F
	vmovups	zmm6{k7}, zmm5	 # AVX512F
	vmovups.s	zmm6{k7}{z}, zmm5	 # AVX512F
	vmovups	zmm6{k7}{z}, zmm5	 # AVX512F
