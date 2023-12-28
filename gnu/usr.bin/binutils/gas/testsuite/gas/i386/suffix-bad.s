	.text
suffix:
	phadddb %xmm0, %xmm1
	phadddd %xmm0, %xmm1
	phadddl %xmm0, %xmm1
	phadddld %xmm0, %xmm1
	phadddq %xmm0, %xmm1
	phaddds %xmm0, %xmm1
	phadddw %xmm0, %xmm1

	.intel_syntax noprefix
	phadddb xmm0, xmm1
	phadddd xmm0, xmm1
	phadddl xmm0, xmm1
	phadddld xmm0, xmm1
	phadddq xmm0, xmm1
	phaddds xmm0, xmm1
	phadddw xmm0, xmm1
