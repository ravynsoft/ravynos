# Encode aligned vector move as unaligned vector move.

	.text
_start:
	movaps %xmm1, %xmm2
	movaps (%eax), %xmm2
	movaps %xmm1, (%eax)
	movapd %xmm1, %xmm2
	movapd (%eax), %xmm2
	movapd %xmm1, (%eax)
	movdqa %xmm1, %xmm2
	movdqa (%eax), %xmm2
	movdqa %xmm1, (%eax)
	vmovaps %xmm1, %xmm2
	vmovaps (%eax), %xmm2
	vmovaps %xmm1, (%eax)
	vmovapd %xmm1, %xmm2
	vmovapd (%eax), %xmm2
	vmovapd %xmm1, (%eax)
	vmovdqa %xmm1, %xmm2
	vmovdqa (%eax), %xmm2
	vmovdqa %xmm1, (%eax)
	vmovaps %xmm1, %xmm2{%k1}
	vmovaps (%eax), %xmm2{%k1}
	vmovaps %xmm1, (%eax){%k1}
	vmovapd %xmm1, %xmm2{%k1}
	vmovapd (%eax), %xmm2{%k1}
	vmovapd %xmm1, (%eax){%k1}
	vmovdqa32 %xmm1, %xmm2
	vmovdqa32 (%eax), %xmm2
	vmovdqa32 %xmm1, (%eax)
	vmovdqa64 %xmm1, %xmm2
	vmovdqa64 (%eax), %xmm2
	vmovdqa64 %xmm1, (%eax)
