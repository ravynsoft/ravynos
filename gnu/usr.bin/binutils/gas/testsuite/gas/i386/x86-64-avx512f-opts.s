# Check 64bit AVX512F instructions

	.allow_index_reg
	.text
_start:

	vmovapd.s	%zmm29, %zmm30	 # AVX512F
	vmovapd	%zmm29, %zmm30	 # AVX512F
	vmovapd.s	%zmm29, %zmm30{%k7}	 # AVX512F
	vmovapd	%zmm29, %zmm30{%k7}	 # AVX512F
	vmovapd.s	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vmovapd	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vmovaps.s	%zmm29, %zmm30	 # AVX512F
	vmovaps	%zmm29, %zmm30	 # AVX512F
	vmovaps.s	%zmm29, %zmm30{%k7}	 # AVX512F
	vmovaps	%zmm29, %zmm30{%k7}	 # AVX512F
	vmovaps.s	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vmovaps	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vmovd.s	%xmm30, %eax	 # AVX512F
	vmovd	%xmm30, %eax	 # AVX512F
	vmovd.s	%xmm30, %ebp	 # AVX512F
	vmovd	%xmm30, %ebp	 # AVX512F
	vmovd.s	%xmm30, %r13d	 # AVX512F
	vmovd	%xmm30, %r13d	 # AVX512F
	vmovdqa32.s	%zmm29, %zmm30	 # AVX512F
	vmovdqa32	%zmm29, %zmm30	 # AVX512F
	vmovdqa32.s	%zmm29, %zmm30{%k7}	 # AVX512F
	vmovdqa32	%zmm29, %zmm30{%k7}	 # AVX512F
	vmovdqa32.s	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vmovdqa32	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vmovdqa64.s	%zmm29, %zmm30	 # AVX512F
	vmovdqa64	%zmm29, %zmm30	 # AVX512F
	vmovdqa64.s	%zmm29, %zmm30{%k7}	 # AVX512F
	vmovdqa64	%zmm29, %zmm30{%k7}	 # AVX512F
	vmovdqa64.s	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vmovdqa64	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vmovdqu32.s	%zmm29, %zmm30	 # AVX512F
	vmovdqu32	%zmm29, %zmm30	 # AVX512F
	vmovdqu32.s	%zmm29, %zmm30{%k7}	 # AVX512F
	vmovdqu32	%zmm29, %zmm30{%k7}	 # AVX512F
	vmovdqu32.s	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vmovdqu32	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vmovdqu64.s	%zmm29, %zmm30	 # AVX512F
	vmovdqu64	%zmm29, %zmm30	 # AVX512F
	vmovdqu64.s	%zmm29, %zmm30{%k7}	 # AVX512F
	vmovdqu64	%zmm29, %zmm30{%k7}	 # AVX512F
	vmovdqu64.s	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vmovdqu64	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vmovq.s	%xmm30, %rax	 # AVX512F
	vmovq	%xmm30, %rax	 # AVX512F
	vmovq.s	%xmm30, %r8	 # AVX512F
	vmovq	%xmm30, %r8	 # AVX512F
	vmovq.s	%xmm29, %xmm30	 # AVX512F
	vmovq	%xmm29, %xmm30	 # AVX512F
	vmovsd.s	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vmovsd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vmovsd.s	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vmovsd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vmovss.s	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vmovss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512F
	vmovss.s	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vmovss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512F
	vmovupd.s	%zmm29, %zmm30	 # AVX512F
	vmovupd	%zmm29, %zmm30	 # AVX512F
	vmovupd.s	%zmm29, %zmm30{%k7}	 # AVX512F
	vmovupd	%zmm29, %zmm30{%k7}	 # AVX512F
	vmovupd.s	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vmovupd	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vmovups.s	%zmm29, %zmm30	 # AVX512F
	vmovups	%zmm29, %zmm30	 # AVX512F
	vmovups.s	%zmm29, %zmm30{%k7}	 # AVX512F
	vmovups	%zmm29, %zmm30{%k7}	 # AVX512F
	vmovups.s	%zmm29, %zmm30{%k7}{z}	 # AVX512F
	vmovups	%zmm29, %zmm30{%k7}{z}	 # AVX512F

	.intel_syntax noprefix
	vmovapd.s	zmm30, zmm29	 # AVX512F
	vmovapd	zmm30, zmm29	 # AVX512F
	vmovapd.s	zmm30{k7}, zmm29	 # AVX512F
	vmovapd	zmm30{k7}, zmm29	 # AVX512F
	vmovapd.s	zmm30{k7}{z}, zmm29	 # AVX512F
	vmovapd	zmm30{k7}{z}, zmm29	 # AVX512F
	vmovaps.s	zmm30, zmm29	 # AVX512F
	vmovaps	zmm30, zmm29	 # AVX512F
	vmovaps.s	zmm30{k7}, zmm29	 # AVX512F
	vmovaps	zmm30{k7}, zmm29	 # AVX512F
	vmovaps.s	zmm30{k7}{z}, zmm29	 # AVX512F
	vmovaps	zmm30{k7}{z}, zmm29	 # AVX512F
	vmovd.s	eax, xmm30	 # AVX512F
	vmovd	eax, xmm30	 # AVX512F
	vmovd.s	ebp, xmm30	 # AVX512F
	vmovd	ebp, xmm30	 # AVX512F
	vmovd.s	r13d, xmm30	 # AVX512F
	vmovd	r13d, xmm30	 # AVX512F
	vmovdqa32.s	zmm30, zmm29	 # AVX512F
	vmovdqa32	zmm30, zmm29	 # AVX512F
	vmovdqa32.s	zmm30{k7}, zmm29	 # AVX512F
	vmovdqa32	zmm30{k7}, zmm29	 # AVX512F
	vmovdqa32.s	zmm30{k7}{z}, zmm29	 # AVX512F
	vmovdqa32	zmm30{k7}{z}, zmm29	 # AVX512F
	vmovdqa64.s	zmm30, zmm29	 # AVX512F
	vmovdqa64	zmm30, zmm29	 # AVX512F
	vmovdqa64.s	zmm30{k7}, zmm29	 # AVX512F
	vmovdqa64	zmm30{k7}, zmm29	 # AVX512F
	vmovdqa64.s	zmm30{k7}{z}, zmm29	 # AVX512F
	vmovdqa64	zmm30{k7}{z}, zmm29	 # AVX512F
	vmovdqu32.s	zmm30, zmm29	 # AVX512F
	vmovdqu32	zmm30, zmm29	 # AVX512F
	vmovdqu32.s	zmm30{k7}, zmm29	 # AVX512F
	vmovdqu32	zmm30{k7}, zmm29	 # AVX512F
	vmovdqu32.s	zmm30{k7}{z}, zmm29	 # AVX512F
	vmovdqu32	zmm30{k7}{z}, zmm29	 # AVX512F
	vmovdqu64.s	zmm30, zmm29	 # AVX512F
	vmovdqu64	zmm30, zmm29	 # AVX512F
	vmovdqu64.s	zmm30{k7}, zmm29	 # AVX512F
	vmovdqu64	zmm30{k7}, zmm29	 # AVX512F
	vmovdqu64.s	zmm30{k7}{z}, zmm29	 # AVX512F
	vmovdqu64	zmm30{k7}{z}, zmm29	 # AVX512F
	vmovq.s	rax, xmm30	 # AVX512F
	vmovq	rax, xmm30	 # AVX512F
	vmovq.s	r8, xmm30	 # AVX512F
	vmovq	r8, xmm30	 # AVX512F
	vmovq.s	xmm30, xmm29	 # AVX512F
	vmovq	xmm30, xmm29	 # AVX512F
	vmovsd.s	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vmovsd	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vmovsd.s	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vmovsd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vmovss.s	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vmovss	xmm30{k7}, xmm29, xmm28	 # AVX512F
	vmovss.s	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vmovss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512F
	vmovupd.s	zmm30, zmm29	 # AVX512F
	vmovupd	zmm30, zmm29	 # AVX512F
	vmovupd.s	zmm30{k7}, zmm29	 # AVX512F
	vmovupd	zmm30{k7}, zmm29	 # AVX512F
	vmovupd.s	zmm30{k7}{z}, zmm29	 # AVX512F
	vmovupd	zmm30{k7}{z}, zmm29	 # AVX512F
	vmovups.s	zmm30, zmm29	 # AVX512F
	vmovups	zmm30, zmm29	 # AVX512F
	vmovups.s	zmm30{k7}, zmm29	 # AVX512F
	vmovups	zmm30{k7}, zmm29	 # AVX512F
	vmovups.s	zmm30{k7}{z}, zmm29	 # AVX512F
	vmovups	zmm30{k7}{z}, zmm29	 # AVX512F
