# Check SSE2 instructions in 16-bit mode

	.code16
	.include "sse2.s"
	.att_syntax prefix

	# also a few SSE* insns
	addps (%bx),%xmm5
	cvtsi2ss %ecx,%xmm3
	cvtss2si %xmm3,%ecx
	cvttss2si %xmm3,%ecx
	extractps $0,%xmm1,%edx
	movmskps %xmm2,%ecx
	pextrb $0,%xmm1,%edx
	pextrd $0,%xmm1,%edx
	pextrw $0,%mm1,%edx
	pextrw $0,%xmm1,%edx
	pinsrb $0,%ecx,%xmm2
	pinsrd $0,%ecx,%xmm2
	pinsrw $0,%ecx,%mm2
	pinsrw $0,%ecx,%xmm2
	pmovmskb %xmm3,%edx

	.intel_syntax noprefix
	cvtsi2ss xmm0, dword ptr [di]
	extractps dword ptr [di], xmm1, 0
	insertps xmm0, dword ptr [di], 0
	pextrd dword ptr [di], xmm1, 0
	pinsrd xmm0, dword ptr [di], 0
