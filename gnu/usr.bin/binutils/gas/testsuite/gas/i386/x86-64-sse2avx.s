# Check 64bit SSE to AVX instructions

	.allow_index_reg
	.text
_start:
# Tests for op mem64
	ldmxcsr (%rcx)
	stmxcsr (%rcx)

# Tests for op xmm/mem128, xmm
	cvtdq2ps %xmm4,%xmm6
	cvtdq2ps (%rcx),%xmm4
	cvtpd2dq %xmm4,%xmm6
	cvtpd2dq (%rcx),%xmm4
	cvtpd2ps %xmm4,%xmm6
	cvtpd2ps (%rcx),%xmm4
	cvtps2dq %xmm4,%xmm6
	cvtps2dq (%rcx),%xmm4
	cvttpd2dq %xmm4,%xmm6
	cvttpd2dq (%rcx),%xmm4
	cvttps2dq %xmm4,%xmm6
	cvttps2dq (%rcx),%xmm4
	movapd %xmm4,%xmm6
	movapd (%rcx),%xmm4
	movaps %xmm4,%xmm6
	movaps (%rcx),%xmm4
	movdqa %xmm4,%xmm6
	movdqa (%rcx),%xmm4
	movdqu %xmm4,%xmm6
	movdqu (%rcx),%xmm4
	movshdup %xmm4,%xmm6
	movshdup (%rcx),%xmm4
	movsldup %xmm4,%xmm6
	movsldup (%rcx),%xmm4
	movupd %xmm4,%xmm6
	movupd (%rcx),%xmm4
	movups %xmm4,%xmm6
	movups (%rcx),%xmm4
	pabsb %xmm4,%xmm6
	pabsb (%rcx),%xmm4
	pabsw %xmm4,%xmm6
	pabsw (%rcx),%xmm4
	pabsd %xmm4,%xmm6
	pabsd (%rcx),%xmm4
	phminposuw %xmm4,%xmm6
	phminposuw (%rcx),%xmm4
	ptest %xmm4,%xmm6
	ptest (%rcx),%xmm4
	rcpps %xmm4,%xmm6
	rcpps (%rcx),%xmm4
	rsqrtps %xmm4,%xmm6
	rsqrtps (%rcx),%xmm4
	sqrtpd %xmm4,%xmm6
	sqrtpd (%rcx),%xmm4
	sqrtps %xmm4,%xmm6
	sqrtps (%rcx),%xmm4
	aesimc %xmm4,%xmm6
	aesimc (%rcx),%xmm4

# Tests for op xmm, xmm/mem128
	movapd %xmm4,%xmm6
	movapd %xmm4,(%rcx)
	movaps %xmm4,%xmm6
	movaps %xmm4,(%rcx)
	movdqa %xmm4,%xmm6
	movdqa %xmm4,(%rcx)
	movdqu %xmm4,%xmm6
	movdqu %xmm4,(%rcx)
	movupd %xmm4,%xmm6
	movupd %xmm4,(%rcx)
	movups %xmm4,%xmm6
	movups %xmm4,(%rcx)

# Tests for op mem128, xmm
	lddqu (%rcx),%xmm4
	movntdqa (%rcx),%xmm4

# Tests for op xmm, mem128
	movntdq %xmm4,(%rcx)
	movntpd %xmm4,(%rcx)
	movntps %xmm4,(%rcx)

# Tests for op xmm/mem128, xmm[, xmm]
	addpd %xmm4,%xmm6
	addpd %xmm14,%xmm6
	addpd (%rcx),%xmm6
	addps %xmm4,%xmm6
	addps %xmm14,%xmm6
	addps (%rcx),%xmm6
	addsubpd %xmm4,%xmm6
	addsubpd (%rcx),%xmm6
	addsubps %xmm4,%xmm6
	addsubps (%rcx),%xmm6
	andnpd %xmm4,%xmm6
	andnpd %xmm14,%xmm6
	andnpd (%rcx),%xmm6
	andnps %xmm4,%xmm6
	andnps %xmm14,%xmm6
	andnps (%rcx),%xmm6
	andpd %xmm4,%xmm6
	andpd %xmm14,%xmm6
	andpd (%rcx),%xmm6
	andps %xmm4,%xmm6
	andps %xmm14,%xmm6
	andps (%rcx),%xmm6
	divpd %xmm4,%xmm6
	divpd (%rcx),%xmm6
	divps %xmm4,%xmm6
	divps (%rcx),%xmm6
	haddpd %xmm4,%xmm6
	haddpd (%rcx),%xmm6
	haddps %xmm4,%xmm6
	haddps (%rcx),%xmm6
	hsubpd %xmm4,%xmm6
	hsubpd (%rcx),%xmm6
	hsubps %xmm4,%xmm6
	hsubps (%rcx),%xmm6
	maxpd %xmm4,%xmm6
	maxpd %xmm14,%xmm6
	maxpd (%rcx),%xmm6
	maxps %xmm4,%xmm6
	maxps %xmm14,%xmm6
	maxps (%rcx),%xmm6
	minpd %xmm4,%xmm6
	minpd %xmm14,%xmm6
	minpd (%rcx),%xmm6
	minps %xmm4,%xmm6
	minps %xmm14,%xmm6
	minps (%rcx),%xmm6
	mulpd %xmm4,%xmm6
	mulpd %xmm14,%xmm6
	mulpd (%rcx),%xmm6
	mulps %xmm4,%xmm6
	mulps %xmm14,%xmm6
	mulps (%rcx),%xmm6
	orpd %xmm4,%xmm6
	orpd %xmm14,%xmm6
	orpd (%rcx),%xmm6
	orps %xmm4,%xmm6
	orps %xmm14,%xmm6
	orps (%rcx),%xmm6
	packsswb %xmm4,%xmm6
	packsswb (%rcx),%xmm6
	packssdw %xmm4,%xmm6
	packssdw (%rcx),%xmm6
	packuswb %xmm4,%xmm6
	packuswb (%rcx),%xmm6
	packusdw %xmm4,%xmm6
	packusdw (%rcx),%xmm6
	paddb %xmm4,%xmm6
	paddb %xmm14,%xmm6
	paddb (%rcx),%xmm6
	paddw %xmm4,%xmm6
	paddw %xmm14,%xmm6
	paddw (%rcx),%xmm6
	paddd %xmm4,%xmm6
	paddd %xmm14,%xmm6
	paddd (%rcx),%xmm6
	paddq %xmm4,%xmm6
	paddq %xmm14,%xmm6
	paddq (%rcx),%xmm6
	paddsb %xmm4,%xmm6
	paddsb %xmm14,%xmm6
	paddsb (%rcx),%xmm6
	paddsw %xmm4,%xmm6
	paddsw %xmm14,%xmm6
	paddsw (%rcx),%xmm6
	paddusb %xmm4,%xmm6
	paddusb %xmm14,%xmm6
	paddusb (%rcx),%xmm6
	paddusw %xmm4,%xmm6
	paddusw %xmm14,%xmm6
	paddusw (%rcx),%xmm6
	pand %xmm4,%xmm6
	pand %xmm14,%xmm6
	pand (%rcx),%xmm6
	pandn %xmm4,%xmm6
	pandn %xmm14,%xmm6
	pandn (%rcx),%xmm6
	pavgb %xmm4,%xmm6
	pavgb %xmm14,%xmm6
	pavgb (%rcx),%xmm6
	pavgw %xmm4,%xmm6
	pavgw %xmm14,%xmm6
	pavgw (%rcx),%xmm6
	pclmullqlqdq %xmm4,%xmm6
	pclmullqlqdq (%rcx),%xmm6
	pclmulhqlqdq %xmm4,%xmm6
	pclmulhqlqdq (%rcx),%xmm6
	pclmullqhqdq %xmm4,%xmm6
	pclmullqhqdq (%rcx),%xmm6
	pclmulhqhqdq %xmm4,%xmm6
	pclmulhqhqdq (%rcx),%xmm6
	pcmpeqb %xmm4,%xmm6
	pcmpeqb %xmm14,%xmm6
	pcmpeqb (%rcx),%xmm6
	pcmpeqw %xmm4,%xmm6
	pcmpeqw %xmm14,%xmm6
	pcmpeqw (%rcx),%xmm6
	pcmpeqd %xmm4,%xmm6
	pcmpeqd %xmm14,%xmm6
	pcmpeqd (%rcx),%xmm6
	pcmpeqq %xmm4,%xmm6
	pcmpeqq (%rcx),%xmm6
	pcmpgtb %xmm4,%xmm6
	pcmpgtb %xmm14,%xmm6
	pcmpgtb (%rcx),%xmm6
	pcmpgtw %xmm4,%xmm6
	pcmpgtw %xmm14,%xmm6
	pcmpgtw (%rcx),%xmm6
	pcmpgtd %xmm4,%xmm6
	pcmpgtd %xmm14,%xmm6
	pcmpgtd (%rcx),%xmm6
	pcmpgtq %xmm4,%xmm6
	pcmpgtq (%rcx),%xmm6
	phaddw %xmm4,%xmm6
	phaddw (%rcx),%xmm6
	phaddd %xmm4,%xmm6
	phaddd (%rcx),%xmm6
	phaddsw %xmm4,%xmm6
	phaddsw (%rcx),%xmm6
	phsubw %xmm4,%xmm6
	phsubw (%rcx),%xmm6
	phsubd %xmm4,%xmm6
	phsubd (%rcx),%xmm6
	phsubsw %xmm4,%xmm6
	phsubsw (%rcx),%xmm6
	pmaddwd %xmm4,%xmm6
	pmaddwd %xmm14,%xmm6
	pmaddwd (%rcx),%xmm6
	pmaddubsw %xmm4,%xmm6
	pmaddubsw (%rcx),%xmm6
	pmaxsb %xmm4,%xmm6
	pmaxsb (%rcx),%xmm6
	pmaxsw %xmm4,%xmm6
	pmaxsw %xmm14,%xmm6
	pmaxsw (%rcx),%xmm6
	pmaxsd %xmm4,%xmm6
	pmaxsd (%rcx),%xmm6
	pmaxub %xmm4,%xmm6
	pmaxub %xmm14,%xmm6
	pmaxub (%rcx),%xmm6
	pmaxuw %xmm4,%xmm6
	pmaxuw (%rcx),%xmm6
	pmaxud %xmm4,%xmm6
	pmaxud (%rcx),%xmm6
	pminsb %xmm4,%xmm6
	pminsb (%rcx),%xmm6
	pminsw %xmm4,%xmm6
	pminsw %xmm14,%xmm6
	pminsw (%rcx),%xmm6
	pminsd %xmm4,%xmm6
	pminsd (%rcx),%xmm6
	pminub %xmm4,%xmm6
	pminub %xmm14,%xmm6
	pminub (%rcx),%xmm6
	pminuw %xmm4,%xmm6
	pminuw (%rcx),%xmm6
	pminud %xmm4,%xmm6
	pminud (%rcx),%xmm6
	pmulhuw %xmm4,%xmm6
	pmulhuw %xmm14,%xmm6
	pmulhuw (%rcx),%xmm6
	pmulhrsw %xmm4,%xmm6
	pmulhrsw (%rcx),%xmm6
	pmulhw %xmm4,%xmm6
	pmulhw %xmm14,%xmm6
	pmulhw (%rcx),%xmm6
	pmullw %xmm4,%xmm6
	pmullw %xmm14,%xmm6
	pmullw (%rcx),%xmm6
	pmulld %xmm4,%xmm6
	pmulld (%rcx),%xmm6
	pmuludq %xmm4,%xmm6
	pmuludq %xmm14,%xmm6
	pmuludq (%rcx),%xmm6
	pmuldq %xmm4,%xmm6
	pmuldq (%rcx),%xmm6
	por %xmm4,%xmm6
	por %xmm14,%xmm6
	por (%rcx),%xmm6
	psadbw %xmm4,%xmm6
	psadbw %xmm14,%xmm6
	psadbw (%rcx),%xmm6
	pshufb %xmm4,%xmm6
	pshufb (%rcx),%xmm6
	psignb %xmm4,%xmm6
	psignb (%rcx),%xmm6
	psignw %xmm4,%xmm6
	psignw (%rcx),%xmm6
	psignd %xmm4,%xmm6
	psignd (%rcx),%xmm6
	psllw %xmm4,%xmm6
	psllw (%rcx),%xmm6
	pslld %xmm4,%xmm6
	pslld (%rcx),%xmm6
	psllq %xmm4,%xmm6
	psllq (%rcx),%xmm6
	psraw %xmm4,%xmm6
	psraw (%rcx),%xmm6
	psrad %xmm4,%xmm6
	psrad (%rcx),%xmm6
	psrlw %xmm4,%xmm6
	psrlw (%rcx),%xmm6
	psrld %xmm4,%xmm6
	psrld (%rcx),%xmm6
	psrlq %xmm4,%xmm6
	psrlq (%rcx),%xmm6
	psubb %xmm4,%xmm6
	psubb (%rcx),%xmm6
	psubw %xmm4,%xmm6
	psubw (%rcx),%xmm6
	psubd %xmm4,%xmm6
	psubd (%rcx),%xmm6
	psubq %xmm4,%xmm6
	psubq (%rcx),%xmm6
	psubsb %xmm4,%xmm6
	psubsb (%rcx),%xmm6
	psubsw %xmm4,%xmm6
	psubsw (%rcx),%xmm6
	psubusb %xmm4,%xmm6
	psubusb (%rcx),%xmm6
	psubusw %xmm4,%xmm6
	psubusw (%rcx),%xmm6
	punpckhbw %xmm4,%xmm6
	punpckhbw (%rcx),%xmm6
	punpckhwd %xmm4,%xmm6
	punpckhwd (%rcx),%xmm6
	punpckhdq %xmm4,%xmm6
	punpckhdq (%rcx),%xmm6
	punpckhqdq %xmm4,%xmm6
	punpckhqdq (%rcx),%xmm6
	punpcklbw %xmm4,%xmm6
	punpcklbw (%rcx),%xmm6
	punpcklwd %xmm4,%xmm6
	punpcklwd (%rcx),%xmm6
	punpckldq %xmm4,%xmm6
	punpckldq (%rcx),%xmm6
	punpcklqdq %xmm4,%xmm6
	punpcklqdq (%rcx),%xmm6
	pxor %xmm4,%xmm6
	pxor %xmm14,%xmm6
	pxor (%rcx),%xmm6
	subpd %xmm4,%xmm6
	subpd (%rcx),%xmm6
	subps %xmm4,%xmm6
	subps (%rcx),%xmm6
	unpckhpd %xmm4,%xmm6
	unpckhpd (%rcx),%xmm6
	unpckhps %xmm4,%xmm6
	unpckhps (%rcx),%xmm6
	unpcklpd %xmm4,%xmm6
	unpcklpd (%rcx),%xmm6
	unpcklps %xmm4,%xmm6
	unpcklps (%rcx),%xmm6
	xorpd %xmm4,%xmm6
	xorpd %xmm14,%xmm6
	xorpd (%rcx),%xmm6
	xorps %xmm4,%xmm6
	xorps %xmm14,%xmm6
	xorps (%rcx),%xmm6
	aesenc %xmm4,%xmm6
	aesenc (%rcx),%xmm6
	aesenclast %xmm4,%xmm6
	aesenclast (%rcx),%xmm6
	aesdec %xmm4,%xmm6
	aesdec (%rcx),%xmm6
	aesdeclast %xmm4,%xmm6
	aesdeclast (%rcx),%xmm6
	cmpeqpd %xmm4,%xmm6
	cmpeqpd %xmm14,%xmm6
	cmpeqpd (%rcx),%xmm6
	cmpeqps %xmm4,%xmm6
	cmpeqps %xmm14,%xmm6
	cmpeqps (%rcx),%xmm6
	cmpltpd %xmm4,%xmm6
	cmpltpd %xmm14,%xmm6
	cmpltpd (%rcx),%xmm6
	cmpltps %xmm4,%xmm6
	cmpltps %xmm14,%xmm6
	cmpltps (%rcx),%xmm6
	cmplepd %xmm4,%xmm6
	cmplepd %xmm14,%xmm6
	cmplepd (%rcx),%xmm6
	cmpleps %xmm4,%xmm6
	cmpleps %xmm14,%xmm6
	cmpleps (%rcx),%xmm6
	cmpunordpd %xmm4,%xmm6
	cmpunordpd %xmm14,%xmm6
	cmpunordpd (%rcx),%xmm6
	cmpunordps %xmm4,%xmm6
	cmpunordps %xmm14,%xmm6
	cmpunordps (%rcx),%xmm6
	cmpneqpd %xmm4,%xmm6
	cmpneqpd %xmm14,%xmm6
	cmpneqpd (%rcx),%xmm6
	cmpneqps %xmm4,%xmm6
	cmpneqps %xmm14,%xmm6
	cmpneqps (%rcx),%xmm6
	cmpnltpd %xmm4,%xmm6
	cmpnltpd %xmm14,%xmm6
	cmpnltpd (%rcx),%xmm6
	cmpnltps %xmm4,%xmm6
	cmpnltps %xmm14,%xmm6
	cmpnltps (%rcx),%xmm6
	cmpnlepd %xmm4,%xmm6
	cmpnlepd %xmm14,%xmm6
	cmpnlepd (%rcx),%xmm6
	cmpnleps %xmm4,%xmm6
	cmpnleps %xmm14,%xmm6
	cmpnleps (%rcx),%xmm6
	cmpordpd %xmm4,%xmm6
	cmpordpd %xmm14,%xmm6
	cmpordpd (%rcx),%xmm6
	cmpordps %xmm4,%xmm6
	cmpordps %xmm14,%xmm6
	cmpordps (%rcx),%xmm6

# Tests for op imm8, xmm/mem128, xmm
	aeskeygenassist $100,%xmm4,%xmm6
	aeskeygenassist $100,(%rcx),%xmm6
	pcmpestri $100,%xmm4,%xmm6
	pcmpestri $100,(%rcx),%xmm6
	pcmpestriq $100,%xmm4,%xmm6
	pcmpestril $100,(%rcx),%xmm6
	pcmpestrm $100,%xmm4,%xmm6
	pcmpestrm $100,(%rcx),%xmm6
	pcmpestrmq $100,%xmm4,%xmm6
	pcmpestrml $100,(%rcx),%xmm6
	pcmpistri $100,%xmm4,%xmm6
	pcmpistri $100,(%rcx),%xmm6
	pcmpistrm $100,%xmm4,%xmm6
	pcmpistrm $100,(%rcx),%xmm6
	pshufd $100,%xmm4,%xmm6
	pshufd $100,(%rcx),%xmm6
	pshufhw $100,%xmm4,%xmm6
	pshufhw $100,(%rcx),%xmm6
	pshuflw $100,%xmm4,%xmm6
	pshuflw $100,(%rcx),%xmm6
	roundpd $100,%xmm4,%xmm6
	roundpd $100,(%rcx),%xmm6
	roundps $100,%xmm4,%xmm6
	roundps $100,(%rcx),%xmm6

# Tests for op imm8, xmm/mem128, xmm[, xmm]
	blendpd $100,%xmm4,%xmm6
	blendpd $100,(%rcx),%xmm6
	blendps $100,%xmm4,%xmm6
	blendps $100,(%rcx),%xmm6
	cmppd $100,%xmm4,%xmm6
	cmppd $100,%xmm14,%xmm6
	cmppd $100,(%rcx),%xmm6
	cmpps $100,%xmm4,%xmm6
	cmpps $100,%xmm14,%xmm6
	cmpps $100,(%rcx),%xmm6
	dppd $100,%xmm4,%xmm6
	dppd $100,(%rcx),%xmm6
	dpps $100,%xmm4,%xmm6
	dpps $100,(%rcx),%xmm6
	mpsadbw $100,%xmm4,%xmm6
	mpsadbw $100,(%rcx),%xmm6
	palignr $100,%xmm4,%xmm6
	palignr $100,(%rcx),%xmm6
	pblendw $100,%xmm4,%xmm6
	pblendw $100,(%rcx),%xmm6
	pclmulqdq $100,%xmm4,%xmm6
	pclmulqdq $100,(%rcx),%xmm6
	shufpd $100,%xmm4,%xmm6
	shufpd $100,(%rcx),%xmm6
	shufps $100,%xmm4,%xmm6
	shufps $100,(%rcx),%xmm6

# Tests for op xmm0, xmm/mem128, xmm[, xmm]
	blendvpd %xmm0,%xmm4,%xmm6
	blendvpd %xmm0,(%rcx),%xmm6
	blendvpd %xmm4,%xmm6
	blendvpd (%rcx),%xmm6
	blendvps %xmm0,%xmm4,%xmm6
	blendvps %xmm0,(%rcx),%xmm6
	blendvps %xmm4,%xmm6
	blendvps (%rcx),%xmm6
	pblendvb %xmm0,%xmm4,%xmm6
	pblendvb %xmm0,(%rcx),%xmm6
	pblendvb %xmm4,%xmm6
	pblendvb (%rcx),%xmm6

# Tests for op xmm/mem64, xmm
	comisd %xmm4,%xmm6
	comisd %xmm14,%xmm6
	comisd (%rcx),%xmm4
	cvtdq2pd %xmm4,%xmm6
	cvtdq2pd (%rcx),%xmm4
	cvtpi2pd (%rcx),%xmm4
	cvtps2pd %xmm4,%xmm6
	cvtps2pd (%rcx),%xmm4
	movddup %xmm4,%xmm6
	movddup (%rcx),%xmm4
	pmovsxbw %xmm4,%xmm6
	pmovsxbw (%rcx),%xmm4
	pmovsxwd %xmm4,%xmm6
	pmovsxwd (%rcx),%xmm4
	pmovsxdq %xmm4,%xmm6
	pmovsxdq (%rcx),%xmm4
	pmovzxbw %xmm4,%xmm6
	pmovzxbw (%rcx),%xmm4
	pmovzxwd %xmm4,%xmm6
	pmovzxwd (%rcx),%xmm4
	pmovzxdq %xmm4,%xmm6
	pmovzxdq (%rcx),%xmm4
	ucomisd %xmm4,%xmm6
	ucomisd %xmm14,%xmm6
	ucomisd (%rcx),%xmm4

# Tests for op mem64, xmm
	movsd (%rcx),%xmm4

# Tests for op xmm, mem64
	movlpd %xmm4,(%rcx)
	movlps %xmm4,(%rcx)
	movhpd %xmm4,(%rcx)
	movhps %xmm4,(%rcx)
	movsd %xmm4,(%rcx)

# Tests for op xmm, regq/mem64
# Tests for op regq/mem64, xmm
	movd %xmm4,%rcx
	movd %rcx,%xmm4
	movq %xmm4,%rcx
	movq %rcx,%xmm4
	movq %xmm4,(%rcx)
	movq (%rcx),%xmm4

# Tests for op xmm/mem64, regl
	cvtsd2si %xmm4,%ecx
	cvtsd2si (%rcx),%ecx
	cvttsd2si %xmm4,%ecx
	cvttsd2si (%rcx),%ecx

# Tests for op xmm/mem64, regq
	cvtsd2si %xmm4,%rcx
	cvtsd2si (%rcx),%rcx
	cvttsd2si %xmm4,%rcx
	cvttsd2si (%rcx),%rcx

# Tests for op regq/mem64, xmm[, xmm]
	cvtsi2sdq %rcx,%xmm4
	cvtsi2sdq (%rcx),%xmm4
	cvtsi2ssq %rcx,%xmm4
	cvtsi2ssq (%rcx),%xmm4

# Tests for op imm8, regq/mem64, xmm[, xmm]
	pinsrq $100,%rcx,%xmm4
	pinsrq $100,(%rcx),%xmm4

# Testsf for op imm8, xmm, regq/mem64
	pextrq $100,%xmm4,%rcx
	pextrq $100,%xmm4,(%rcx)

# Tests for op mem64, xmm[, xmm]
	movlpd (%rcx),%xmm4
	movlps (%rcx),%xmm4
	movhpd (%rcx),%xmm4
	movhps (%rcx),%xmm4

# Tests for op imm8, xmm/mem64, xmm[, xmm]
	cmpsd $100,%xmm4,%xmm6
	cmpsd $100,%xmm14,%xmm6
	cmpsd $100,(%rcx),%xmm6
	roundsd $100,%xmm4,%xmm6
	roundsd $100,(%rcx),%xmm6

# Tests for op xmm/mem64, xmm[, xmm]
	addsd %xmm4,%xmm6
	addsd %xmm14,%xmm6
	addsd (%rcx),%xmm6
	cvtsd2ss %xmm4,%xmm6
	cvtsd2ss (%rcx),%xmm6
	divsd %xmm4,%xmm6
	divsd (%rcx),%xmm6
	maxsd %xmm4,%xmm6
	maxsd %xmm14,%xmm6
	maxsd (%rcx),%xmm6
	minsd %xmm4,%xmm6
	minsd %xmm14,%xmm6
	minsd (%rcx),%xmm6
	mulsd %xmm4,%xmm6
	mulsd %xmm14,%xmm6
	mulsd (%rcx),%xmm6
	sqrtsd %xmm4,%xmm6
	sqrtsd (%rcx),%xmm6
	subsd %xmm4,%xmm6
	subsd (%rcx),%xmm6
	cmpeqsd %xmm4,%xmm6
	cmpeqsd %xmm14,%xmm6
	cmpeqsd (%rcx),%xmm6
	cmpltsd %xmm4,%xmm6
	cmpltsd %xmm14,%xmm6
	cmpltsd (%rcx),%xmm6
	cmplesd %xmm4,%xmm6
	cmplesd %xmm14,%xmm6
	cmplesd (%rcx),%xmm6
	cmpunordsd %xmm4,%xmm6
	cmpunordsd %xmm14,%xmm6
	cmpunordsd (%rcx),%xmm6
	cmpneqsd %xmm4,%xmm6
	cmpneqsd %xmm14,%xmm6
	cmpneqsd (%rcx),%xmm6
	cmpnltsd %xmm4,%xmm6
	cmpnltsd %xmm14,%xmm6
	cmpnltsd (%rcx),%xmm6
	cmpnlesd %xmm4,%xmm6
	cmpnlesd %xmm14,%xmm6
	cmpnlesd (%rcx),%xmm6
	cmpordsd %xmm4,%xmm6
	cmpordsd %xmm14,%xmm6
	cmpordsd (%rcx),%xmm6

# Tests for op xmm/mem32, xmm[, xmm]
	addss %xmm4,%xmm6
	addss %xmm14,%xmm6
	addss (%rcx),%xmm6
	cvtss2sd %xmm4,%xmm6
	cvtss2sd (%rcx),%xmm6
	divss %xmm4,%xmm6
	divss (%rcx),%xmm6
	maxss %xmm4,%xmm6
	maxss %xmm14,%xmm6
	maxss (%rcx),%xmm6
	minss %xmm4,%xmm6
	minss %xmm14,%xmm6
	minss (%rcx),%xmm6
	mulss %xmm4,%xmm6
	mulss %xmm14,%xmm6
	mulss (%rcx),%xmm6
	rcpss %xmm4,%xmm6
	rcpss (%rcx),%xmm6
	rsqrtss %xmm4,%xmm6
	rsqrtss (%rcx),%xmm6
	sqrtss %xmm4,%xmm6
	sqrtss (%rcx),%xmm6
	subss %xmm4,%xmm6
	subss (%rcx),%xmm6
	cmpeqss %xmm4,%xmm6
	cmpeqss %xmm14,%xmm6
	cmpeqss (%rcx),%xmm6
	cmpltss %xmm4,%xmm6
	cmpltss %xmm14,%xmm6
	cmpltss (%rcx),%xmm6
	cmpless %xmm4,%xmm6
	cmpless %xmm14,%xmm6
	cmpless (%rcx),%xmm6
	cmpunordss %xmm4,%xmm6
	cmpunordss %xmm14,%xmm6
	cmpunordss (%rcx),%xmm6
	cmpneqss %xmm4,%xmm6
	cmpneqss %xmm14,%xmm6
	cmpneqss (%rcx),%xmm6
	cmpnltss %xmm4,%xmm6
	cmpnltss %xmm14,%xmm6
	cmpnltss (%rcx),%xmm6
	cmpnless %xmm4,%xmm6
	cmpnless %xmm14,%xmm6
	cmpnless (%rcx),%xmm6
	cmpordss %xmm4,%xmm6
	cmpordss %xmm14,%xmm6
	cmpordss (%rcx),%xmm6

# Tests for op xmm/mem32, xmm
	comiss %xmm4,%xmm6
	comiss %xmm14,%xmm6
	comiss (%rcx),%xmm4
	pmovsxbd %xmm4,%xmm6
	pmovsxbd (%rcx),%xmm4
	pmovsxwq %xmm4,%xmm6
	pmovsxwq (%rcx),%xmm4
	pmovzxbd %xmm4,%xmm6
	pmovzxbd (%rcx),%xmm4
	pmovzxwq %xmm4,%xmm6
	pmovzxwq (%rcx),%xmm4
	ucomiss %xmm4,%xmm6
	ucomiss %xmm14,%xmm6
	ucomiss (%rcx),%xmm4

# Tests for op mem32, xmm
	movss (%rcx),%xmm4

# Tests for op xmm, mem32
	movss %xmm4,(%rcx)

# Tests for op xmm, regl/mem32
# Tests for op regl/mem32, xmm
	movd %xmm4,%ecx
	movd %xmm4,(%rcx)
	movd %ecx,%xmm4
	movd (%rcx),%xmm4

# Tests for op xmm/mem32, regl
	cvtss2si %xmm4,%ecx
	cvtss2si (%rcx),%ecx
	cvttss2si %xmm4,%ecx
	cvttss2si (%rcx),%ecx

# Tests for op xmm/mem32, regq
	cvtss2si %xmm4,%rcx
	cvtss2si (%rcx),%rcx
	cvttss2si %xmm4,%rcx
	cvttss2si (%rcx),%rcx

# Tests for op xmm, regq
	movmskpd %xmm4,%rcx
	movmskps %xmm4,%rcx
	pmovmskb %xmm4,%rcx

# Tests for op imm8, xmm, regq/mem32
	extractps $100,%xmm4,%rcx
	extractps $100,%xmm4,(%rcx)
# Tests for op imm8, xmm, regl/mem32
	pextrd $100,%xmm4,%ecx
	pextrd $100,%xmm4,(%rcx)
	extractps $100,%xmm4,%ecx
	extractps $100,%xmm4,(%rcx)

# Tests for op regl/mem32, xmm[, xmm]
	cvtsi2sd %ecx,%xmm4
	cvtsi2sdl (%rcx),%xmm4
	cvtsi2ss %ecx,%xmm4
	cvtsi2ssl (%rcx),%xmm4

# Tests for op imm8, xmm/mem32, xmm[, xmm]
	cmpss $100,%xmm4,%xmm6
	cmpss $100,%xmm14,%xmm6
	cmpss $100,(%rcx),%xmm6
	insertps $100,%xmm4,%xmm6
	insertps $100,(%rcx),%xmm6
	roundss $100,%xmm4,%xmm6
	roundss $100,(%rcx),%xmm6

# Tests for op xmm/m16, xmm
	pmovsxbq %xmm4,%xmm6
	pmovsxbq (%rcx),%xmm4
	pmovzxbq %xmm4,%xmm6
	pmovzxbq (%rcx),%xmm4

# Tests for op imm8, xmm, regl/mem16
	pextrw $100,%xmm4,%ecx
	pextrw $100,%xmm4,(%rcx)

# Tests for op imm8, xmm, regq/mem16
	pextrw $100,%xmm4,%rcx
	pextrw $100,%xmm4,(%rcx)

# Tests for op imm8, regl/mem16, xmm[, xmm]
	pinsrw $100,%ecx,%xmm4
	pinsrw $100,(%rcx),%xmm4


	pinsrw $100,%rcx,%xmm4
	pinsrw $100,(%rcx),%xmm4

# Tests for op imm8, xmm, regl/mem8
	pextrb $100,%xmm4,%ecx
	pextrb $100,%xmm4,(%rcx)

# Tests for op imm8, regl/mem8, xmm[, xmm]
	pinsrb $100,%ecx,%xmm4
	pinsrb $100,(%rcx),%xmm4

# Tests for op imm8, xmm, regq
	pextrw $100,%xmm4,%rcx
# Tests for op imm8, xmm, regq/mem8
	pextrb $100,%xmm4,%rcx
	pextrb $100,%xmm4,(%rcx)

# Tests for op imm8, regl/mem8, xmm[, xmm]
	pinsrb $100,%ecx,%xmm4
	pinsrb $100,(%rcx),%xmm4

# Tests for op xmm, xmm
	maskmovdqu %xmm4,%xmm6
	movq %xmm4,%xmm6

# Tests for op xmm, regl
	movmskpd %xmm4,%ecx
	movmskps %xmm4,%ecx
	pmovmskb %xmm4,%ecx
# Tests for op xmm, xmm[, xmm]
	movhlps %xmm4,%xmm6
	movlhps %xmm4,%xmm6
	movsd %xmm4,%xmm6
	movss %xmm4,%xmm6

# Tests for op imm8, xmm[, xmm]
	pslld $100,%xmm4
	pslldq $100,%xmm4
	psllq $100,%xmm4
	psllw $100,%xmm4
	psrad $100,%xmm4
	psraw $100,%xmm4
	psrld $100,%xmm4
	psrldq $100,%xmm4
	psrlq $100,%xmm4
	psrlw $100,%xmm4

# Tests for op imm8, xmm, regl
	pextrw $100,%xmm4,%ecx

# Tests for REX prefix conversion
	{rex} addps %xmm0, %xmm1
	{rex} addps (%rax,%rax), %xmm1
	rex addps %xmm0, %xmm1
	rex addps (%rax,%rax), %xmm1
	rexx addps %xmm0, %xmm1
	rexx addps (%rax,%rax), %xmm1
	rexy addps %xmm0, %xmm1
	rexy addps (%rax,%rax), %xmm1
	rexz addps %xmm0, %xmm1
	rexz addps (%rax,%rax), %xmm1

	{load} rexx movss %xmm0, %xmm1
	{load} rexz movss %xmm0, %xmm1

	{store} rexx movss %xmm0, %xmm1
	{store} rexz movss %xmm0, %xmm1

	rexz psllw $0, %xmm0

	rexx pextrw $0, %xmm0, %ecx
	rexz pextrw $0, %xmm0, %ecx

	rexx pextrb $0, %xmm0, %ecx
	rexz pextrb $0, %xmm0, %ecx

	rexx blendvps %xmm0, %xmm0, %xmm1
	rexz blendvps %xmm0, %xmm0, %xmm1

	rexx blendvps %xmm0, %xmm1
	rexz blendvps %xmm0, %xmm1

	rex64 cvtsi2sd (%rax), %xmm0
	rex64 cvtsi2ss (%rax), %xmm0

	rex64 pcmpestri $0, %xmm0, %xmm0
	rex64 pcmpestrm $0, %xmm0, %xmm0


	.intel_syntax noprefix
# Tests for op mem64
	ldmxcsr DWORD PTR [rcx]
	stmxcsr DWORD PTR [rcx]

# Tests for op xmm/mem128, xmm
	cvtdq2ps xmm6,xmm4
	cvtdq2ps xmm4,XMMWORD PTR [rcx]
	cvtpd2dq xmm6,xmm4
	cvtpd2dq xmm4,XMMWORD PTR [rcx]
	cvtpd2ps xmm6,xmm4
	cvtpd2ps xmm4,XMMWORD PTR [rcx]
	cvtps2dq xmm6,xmm4
	cvtps2dq xmm4,XMMWORD PTR [rcx]
	cvttpd2dq xmm6,xmm4
	cvttpd2dq xmm4,XMMWORD PTR [rcx]
	cvttps2dq xmm6,xmm4
	cvttps2dq xmm4,XMMWORD PTR [rcx]
	movapd xmm6,xmm4
	movapd xmm4,XMMWORD PTR [rcx]
	movaps xmm6,xmm4
	movaps xmm4,XMMWORD PTR [rcx]
	movdqa xmm6,xmm4
	movdqa xmm4,XMMWORD PTR [rcx]
	movdqu xmm6,xmm4
	movdqu xmm4,XMMWORD PTR [rcx]
	movshdup xmm6,xmm4
	movshdup xmm4,XMMWORD PTR [rcx]
	movsldup xmm6,xmm4
	movsldup xmm4,XMMWORD PTR [rcx]
	movupd xmm6,xmm4
	movupd xmm4,XMMWORD PTR [rcx]
	movups xmm6,xmm4
	movups xmm4,XMMWORD PTR [rcx]
	pabsb xmm6,xmm4
	pabsb xmm4,XMMWORD PTR [rcx]
	pabsw xmm6,xmm4
	pabsw xmm4,XMMWORD PTR [rcx]
	pabsd xmm6,xmm4
	pabsd xmm4,XMMWORD PTR [rcx]
	phminposuw xmm6,xmm4
	phminposuw xmm4,XMMWORD PTR [rcx]
	ptest xmm6,xmm4
	ptest xmm4,XMMWORD PTR [rcx]
	rcpps xmm6,xmm4
	rcpps xmm4,XMMWORD PTR [rcx]
	rsqrtps xmm6,xmm4
	rsqrtps xmm4,XMMWORD PTR [rcx]
	sqrtpd xmm6,xmm4
	sqrtpd xmm4,XMMWORD PTR [rcx]
	sqrtps xmm6,xmm4
	sqrtps xmm4,XMMWORD PTR [rcx]
	aesimc xmm6,xmm4
	aesimc xmm4,XMMWORD PTR [rcx]

# Tests for op xmm, xmm/mem128
	movapd xmm6,xmm4
	movapd XMMWORD PTR [rcx],xmm4
	movaps xmm6,xmm4
	movaps XMMWORD PTR [rcx],xmm4
	movdqa xmm6,xmm4
	movdqa XMMWORD PTR [rcx],xmm4
	movdqu xmm6,xmm4
	movdqu XMMWORD PTR [rcx],xmm4
	movupd xmm6,xmm4
	movupd XMMWORD PTR [rcx],xmm4
	movups xmm6,xmm4
	movups XMMWORD PTR [rcx],xmm4

# Tests for op mem128, xmm
	lddqu xmm4,XMMWORD PTR [rcx]
	movntdqa xmm4,XMMWORD PTR [rcx]

# Tests for op xmm, mem128
	movntdq XMMWORD PTR [rcx],xmm4
	movntpd XMMWORD PTR [rcx],xmm4
	movntps XMMWORD PTR [rcx],xmm4

# Tests for op xmm/mem128, xmm[, xmm]
	addpd xmm6,xmm4
	addpd xmm6,XMMWORD PTR [rcx]
	addps xmm6,xmm4
	addps xmm6,XMMWORD PTR [rcx]
	addsubpd xmm6,xmm4
	addsubpd xmm6,XMMWORD PTR [rcx]
	addsubps xmm6,xmm4
	addsubps xmm6,XMMWORD PTR [rcx]
	andnpd xmm6,xmm4
	andnpd xmm6,XMMWORD PTR [rcx]
	andnps xmm6,xmm4
	andnps xmm6,XMMWORD PTR [rcx]
	andpd xmm6,xmm4
	andpd xmm6,XMMWORD PTR [rcx]
	andps xmm6,xmm4
	andps xmm6,XMMWORD PTR [rcx]
	divpd xmm6,xmm4
	divpd xmm6,XMMWORD PTR [rcx]
	divps xmm6,xmm4
	divps xmm6,XMMWORD PTR [rcx]
	haddpd xmm6,xmm4
	haddpd xmm6,XMMWORD PTR [rcx]
	haddps xmm6,xmm4
	haddps xmm6,XMMWORD PTR [rcx]
	hsubpd xmm6,xmm4
	hsubpd xmm6,XMMWORD PTR [rcx]
	hsubps xmm6,xmm4
	hsubps xmm6,XMMWORD PTR [rcx]
	maxpd xmm6,xmm4
	maxpd xmm6,XMMWORD PTR [rcx]
	maxps xmm6,xmm4
	maxps xmm6,XMMWORD PTR [rcx]
	minpd xmm6,xmm4
	minpd xmm6,XMMWORD PTR [rcx]
	minps xmm6,xmm4
	minps xmm6,XMMWORD PTR [rcx]
	mulpd xmm6,xmm4
	mulpd xmm6,XMMWORD PTR [rcx]
	mulps xmm6,xmm4
	mulps xmm6,XMMWORD PTR [rcx]
	orpd xmm6,xmm4
	orpd xmm6,XMMWORD PTR [rcx]
	orps xmm6,xmm4
	orps xmm6,XMMWORD PTR [rcx]
	packsswb xmm6,xmm4
	packsswb xmm6,XMMWORD PTR [rcx]
	packssdw xmm6,xmm4
	packssdw xmm6,XMMWORD PTR [rcx]
	packuswb xmm6,xmm4
	packuswb xmm6,XMMWORD PTR [rcx]
	packusdw xmm6,xmm4
	packusdw xmm6,XMMWORD PTR [rcx]
	paddb xmm6,xmm4
	paddb xmm6,XMMWORD PTR [rcx]
	paddw xmm6,xmm4
	paddw xmm6,XMMWORD PTR [rcx]
	paddd xmm6,xmm4
	paddd xmm6,XMMWORD PTR [rcx]
	paddq xmm6,xmm4
	paddq xmm6,XMMWORD PTR [rcx]
	paddsb xmm6,xmm4
	paddsb xmm6,XMMWORD PTR [rcx]
	paddsw xmm6,xmm4
	paddsw xmm6,XMMWORD PTR [rcx]
	paddusb xmm6,xmm4
	paddusb xmm6,XMMWORD PTR [rcx]
	paddusw xmm6,xmm4
	paddusw xmm6,XMMWORD PTR [rcx]
	pand xmm6,xmm4
	pand xmm6,XMMWORD PTR [rcx]
	pandn xmm6,xmm4
	pandn xmm6,XMMWORD PTR [rcx]
	pavgb xmm6,xmm4
	pavgb xmm6,XMMWORD PTR [rcx]
	pavgw xmm6,xmm4
	pavgw xmm6,XMMWORD PTR [rcx]
	pclmullqlqdq xmm6,xmm4
	pclmullqlqdq xmm6,XMMWORD PTR [rcx]
	pclmulhqlqdq xmm6,xmm4
	pclmulhqlqdq xmm6,XMMWORD PTR [rcx]
	pclmullqhqdq xmm6,xmm4
	pclmullqhqdq xmm6,XMMWORD PTR [rcx]
	pclmulhqhqdq xmm6,xmm4
	pclmulhqhqdq xmm6,XMMWORD PTR [rcx]
	pcmpeqb xmm6,xmm4
	pcmpeqb xmm6,XMMWORD PTR [rcx]
	pcmpeqw xmm6,xmm4
	pcmpeqw xmm6,XMMWORD PTR [rcx]
	pcmpeqd xmm6,xmm4
	pcmpeqd xmm6,XMMWORD PTR [rcx]
	pcmpeqq xmm6,xmm4
	pcmpeqq xmm6,XMMWORD PTR [rcx]
	pcmpgtb xmm6,xmm4
	pcmpgtb xmm6,XMMWORD PTR [rcx]
	pcmpgtw xmm6,xmm4
	pcmpgtw xmm6,XMMWORD PTR [rcx]
	pcmpgtd xmm6,xmm4
	pcmpgtd xmm6,XMMWORD PTR [rcx]
	pcmpgtq xmm6,xmm4
	pcmpgtq xmm6,XMMWORD PTR [rcx]
	phaddw xmm6,xmm4
	phaddw xmm6,XMMWORD PTR [rcx]
	phaddd xmm6,xmm4
	phaddd xmm6,XMMWORD PTR [rcx]
	phaddsw xmm6,xmm4
	phaddsw xmm6,XMMWORD PTR [rcx]
	phsubw xmm6,xmm4
	phsubw xmm6,XMMWORD PTR [rcx]
	phsubd xmm6,xmm4
	phsubd xmm6,XMMWORD PTR [rcx]
	phsubsw xmm6,xmm4
	phsubsw xmm6,XMMWORD PTR [rcx]
	pmaddwd xmm6,xmm4
	pmaddwd xmm6,XMMWORD PTR [rcx]
	pmaddubsw xmm6,xmm4
	pmaddubsw xmm6,XMMWORD PTR [rcx]
	pmaxsb xmm6,xmm4
	pmaxsb xmm6,XMMWORD PTR [rcx]
	pmaxsw xmm6,xmm4
	pmaxsw xmm6,XMMWORD PTR [rcx]
	pmaxsd xmm6,xmm4
	pmaxsd xmm6,XMMWORD PTR [rcx]
	pmaxub xmm6,xmm4
	pmaxub xmm6,XMMWORD PTR [rcx]
	pmaxuw xmm6,xmm4
	pmaxuw xmm6,XMMWORD PTR [rcx]
	pmaxud xmm6,xmm4
	pmaxud xmm6,XMMWORD PTR [rcx]
	pminsb xmm6,xmm4
	pminsb xmm6,XMMWORD PTR [rcx]
	pminsw xmm6,xmm4
	pminsw xmm6,XMMWORD PTR [rcx]
	pminsd xmm6,xmm4
	pminsd xmm6,XMMWORD PTR [rcx]
	pminub xmm6,xmm4
	pminub xmm6,XMMWORD PTR [rcx]
	pminuw xmm6,xmm4
	pminuw xmm6,XMMWORD PTR [rcx]
	pminud xmm6,xmm4
	pminud xmm6,XMMWORD PTR [rcx]
	pmulhuw xmm6,xmm4
	pmulhuw xmm6,XMMWORD PTR [rcx]
	pmulhrsw xmm6,xmm4
	pmulhrsw xmm6,XMMWORD PTR [rcx]
	pmulhw xmm6,xmm4
	pmulhw xmm6,XMMWORD PTR [rcx]
	pmullw xmm6,xmm4
	pmullw xmm6,XMMWORD PTR [rcx]
	pmulld xmm6,xmm4
	pmulld xmm6,XMMWORD PTR [rcx]
	pmuludq xmm6,xmm4
	pmuludq xmm6,XMMWORD PTR [rcx]
	pmuldq xmm6,xmm4
	pmuldq xmm6,XMMWORD PTR [rcx]
	por xmm6,xmm4
	por xmm6,XMMWORD PTR [rcx]
	psadbw xmm6,xmm4
	psadbw xmm6,XMMWORD PTR [rcx]
	pshufb xmm6,xmm4
	pshufb xmm6,XMMWORD PTR [rcx]
	psignb xmm6,xmm4
	psignb xmm6,XMMWORD PTR [rcx]
	psignw xmm6,xmm4
	psignw xmm6,XMMWORD PTR [rcx]
	psignd xmm6,xmm4
	psignd xmm6,XMMWORD PTR [rcx]
	psllw xmm6,xmm4
	psllw xmm6,XMMWORD PTR [rcx]
	pslld xmm6,xmm4
	pslld xmm6,XMMWORD PTR [rcx]
	psllq xmm6,xmm4
	psllq xmm6,XMMWORD PTR [rcx]
	psraw xmm6,xmm4
	psraw xmm6,XMMWORD PTR [rcx]
	psrad xmm6,xmm4
	psrad xmm6,XMMWORD PTR [rcx]
	psrlw xmm6,xmm4
	psrlw xmm6,XMMWORD PTR [rcx]
	psrld xmm6,xmm4
	psrld xmm6,XMMWORD PTR [rcx]
	psrlq xmm6,xmm4
	psrlq xmm6,XMMWORD PTR [rcx]
	psubb xmm6,xmm4
	psubb xmm6,XMMWORD PTR [rcx]
	psubw xmm6,xmm4
	psubw xmm6,XMMWORD PTR [rcx]
	psubd xmm6,xmm4
	psubd xmm6,XMMWORD PTR [rcx]
	psubq xmm6,xmm4
	psubq xmm6,XMMWORD PTR [rcx]
	psubsb xmm6,xmm4
	psubsb xmm6,XMMWORD PTR [rcx]
	psubsw xmm6,xmm4
	psubsw xmm6,XMMWORD PTR [rcx]
	psubusb xmm6,xmm4
	psubusb xmm6,XMMWORD PTR [rcx]
	psubusw xmm6,xmm4
	psubusw xmm6,XMMWORD PTR [rcx]
	punpckhbw xmm6,xmm4
	punpckhbw xmm6,XMMWORD PTR [rcx]
	punpckhwd xmm6,xmm4
	punpckhwd xmm6,XMMWORD PTR [rcx]
	punpckhdq xmm6,xmm4
	punpckhdq xmm6,XMMWORD PTR [rcx]
	punpckhqdq xmm6,xmm4
	punpckhqdq xmm6,XMMWORD PTR [rcx]
	punpcklbw xmm6,xmm4
	punpcklbw xmm6,XMMWORD PTR [rcx]
	punpcklwd xmm6,xmm4
	punpcklwd xmm6,XMMWORD PTR [rcx]
	punpckldq xmm6,xmm4
	punpckldq xmm6,XMMWORD PTR [rcx]
	punpcklqdq xmm6,xmm4
	punpcklqdq xmm6,XMMWORD PTR [rcx]
	pxor xmm6,xmm4
	pxor xmm6,XMMWORD PTR [rcx]
	subpd xmm6,xmm4
	subpd xmm6,XMMWORD PTR [rcx]
	subps xmm6,xmm4
	subps xmm6,XMMWORD PTR [rcx]
	unpckhpd xmm6,xmm4
	unpckhpd xmm6,XMMWORD PTR [rcx]
	unpckhps xmm6,xmm4
	unpckhps xmm6,XMMWORD PTR [rcx]
	unpcklpd xmm6,xmm4
	unpcklpd xmm6,XMMWORD PTR [rcx]
	unpcklps xmm6,xmm4
	unpcklps xmm6,XMMWORD PTR [rcx]
	xorpd xmm6,xmm4
	xorpd xmm6,XMMWORD PTR [rcx]
	xorps xmm6,xmm4
	xorps xmm6,XMMWORD PTR [rcx]
	aesenc xmm6,xmm4
	aesenc xmm6,XMMWORD PTR [rcx]
	aesenclast xmm6,xmm4
	aesenclast xmm6,XMMWORD PTR [rcx]
	aesdec xmm6,xmm4
	aesdec xmm6,XMMWORD PTR [rcx]
	aesdeclast xmm6,xmm4
	aesdeclast xmm6,XMMWORD PTR [rcx]
	cmpeqpd xmm6,xmm4
	cmpeqpd xmm6,XMMWORD PTR [rcx]
	cmpeqps xmm6,xmm4
	cmpeqps xmm6,XMMWORD PTR [rcx]
	cmpltpd xmm6,xmm4
	cmpltpd xmm6,XMMWORD PTR [rcx]
	cmpltps xmm6,xmm4
	cmpltps xmm6,XMMWORD PTR [rcx]
	cmplepd xmm6,xmm4
	cmplepd xmm6,XMMWORD PTR [rcx]
	cmpleps xmm6,xmm4
	cmpleps xmm6,XMMWORD PTR [rcx]
	cmpunordpd xmm6,xmm4
	cmpunordpd xmm6,XMMWORD PTR [rcx]
	cmpunordps xmm6,xmm4
	cmpunordps xmm6,XMMWORD PTR [rcx]
	cmpneqpd xmm6,xmm4
	cmpneqpd xmm6,XMMWORD PTR [rcx]
	cmpneqps xmm6,xmm4
	cmpneqps xmm6,XMMWORD PTR [rcx]
	cmpnltpd xmm6,xmm4
	cmpnltpd xmm6,XMMWORD PTR [rcx]
	cmpnltps xmm6,xmm4
	cmpnltps xmm6,XMMWORD PTR [rcx]
	cmpnlepd xmm6,xmm4
	cmpnlepd xmm6,XMMWORD PTR [rcx]
	cmpnleps xmm6,xmm4
	cmpnleps xmm6,XMMWORD PTR [rcx]
	cmpordpd xmm6,xmm4
	cmpordpd xmm6,XMMWORD PTR [rcx]
	cmpordps xmm6,xmm4
	cmpordps xmm6,XMMWORD PTR [rcx]

# Tests for op imm8, xmm/mem128, xmm
	aeskeygenassist xmm6,xmm4,100
	aeskeygenassist xmm6,XMMWORD PTR [rcx],100
	pcmpestri xmm6,xmm4,100
	pcmpestri xmm6,XMMWORD PTR [rcx],100
	pcmpestrm xmm6,xmm4,100
	pcmpestrm xmm6,XMMWORD PTR [rcx],100
	pcmpistri xmm6,xmm4,100
	pcmpistri xmm6,XMMWORD PTR [rcx],100
	pcmpistrm xmm6,xmm4,100
	pcmpistrm xmm6,XMMWORD PTR [rcx],100
	pshufd xmm6,xmm4,100
	pshufd xmm6,XMMWORD PTR [rcx],100
	pshufhw xmm6,xmm4,100
	pshufhw xmm6,XMMWORD PTR [rcx],100
	pshuflw xmm6,xmm4,100
	pshuflw xmm6,XMMWORD PTR [rcx],100
	roundpd xmm6,xmm4,100
	roundpd xmm6,XMMWORD PTR [rcx],100
	roundps xmm6,xmm4,100
	roundps xmm6,XMMWORD PTR [rcx],100

# Tests for op imm8, xmm/mem128, xmm[, xmm]
	blendpd xmm6,xmm4,100
	blendpd xmm6,XMMWORD PTR [rcx],100
	blendps xmm6,xmm4,100
	blendps xmm6,XMMWORD PTR [rcx],100
	cmppd xmm6,xmm4,100
	cmppd xmm6,XMMWORD PTR [rcx],100
	cmpps xmm6,xmm4,100
	cmpps xmm6,XMMWORD PTR [rcx],100
	dppd xmm6,xmm4,100
	dppd xmm6,XMMWORD PTR [rcx],100
	dpps xmm6,xmm4,100
	dpps xmm6,XMMWORD PTR [rcx],100
	mpsadbw xmm6,xmm4,100
	mpsadbw xmm6,XMMWORD PTR [rcx],100
	palignr xmm6,xmm4,100
	palignr xmm6,XMMWORD PTR [rcx],100
	pblendw xmm6,xmm4,100
	pblendw xmm6,XMMWORD PTR [rcx],100
	pclmulqdq xmm6,xmm4,100
	pclmulqdq xmm6,XMMWORD PTR [rcx],100
	shufpd xmm6,xmm4,100
	shufpd xmm6,XMMWORD PTR [rcx],100
	shufps xmm6,xmm4,100
	shufps xmm6,XMMWORD PTR [rcx],100

# Tests for op xmm0, xmm/mem128, xmm[, xmm]
	blendvpd xmm6,xmm4,xmm0
	blendvpd xmm6,XMMWORD PTR [rcx],xmm0
	blendvpd xmm6,xmm4
	blendvpd xmm6,XMMWORD PTR [rcx]
	blendvps xmm6,xmm4,xmm0
	blendvps xmm6,XMMWORD PTR [rcx],xmm0
	blendvps xmm6,xmm4
	blendvps xmm6,XMMWORD PTR [rcx]
	pblendvb xmm6,xmm4,xmm0
	pblendvb xmm6,XMMWORD PTR [rcx],xmm0
	pblendvb xmm6,xmm4
	pblendvb xmm6,XMMWORD PTR [rcx]

# Tests for op xmm/mem64, xmm
	comisd xmm6,xmm4
	comisd xmm4,QWORD PTR [rcx]
	cvtdq2pd xmm6,xmm4
	cvtdq2pd xmm4,QWORD PTR [rcx]
	cvtpi2pd xmm4,QWORD PTR [rcx]
	cvtps2pd xmm6,xmm4
	cvtps2pd xmm4,QWORD PTR [rcx]
	movddup xmm6,xmm4
	movddup xmm4,QWORD PTR [rcx]
	pmovsxbw xmm6,xmm4
	pmovsxbw xmm4,QWORD PTR [rcx]
	pmovsxwd xmm6,xmm4
	pmovsxwd xmm4,QWORD PTR [rcx]
	pmovsxdq xmm6,xmm4
	pmovsxdq xmm4,QWORD PTR [rcx]
	pmovzxbw xmm6,xmm4
	pmovzxbw xmm4,QWORD PTR [rcx]
	pmovzxwd xmm6,xmm4
	pmovzxwd xmm4,QWORD PTR [rcx]
	pmovzxdq xmm6,xmm4
	pmovzxdq xmm4,QWORD PTR [rcx]
	ucomisd xmm6,xmm4
	ucomisd xmm4,QWORD PTR [rcx]

# Tests for op mem64, xmm
	movsd xmm4,QWORD PTR [rcx]

# Tests for op xmm, mem64
	movlpd QWORD PTR [rcx],xmm4
	movlps QWORD PTR [rcx],xmm4
	movhpd QWORD PTR [rcx],xmm4
	movhps QWORD PTR [rcx],xmm4
	movsd QWORD PTR [rcx],xmm4

# Tests for op xmm, regq/mem64
# Tests for op regq/mem64, xmm
	movd rcx,xmm4
	movd xmm4,rcx
	movq rcx,xmm4
	movq xmm4,rcx
	movq QWORD PTR [rcx],xmm4
	movq xmm4,QWORD PTR [rcx]

# Tests for op xmm/mem64, regl
	cvtsd2si ecx,xmm4
	cvtsd2si ecx,QWORD PTR [rcx]
	cvttsd2si ecx,xmm4
	cvttsd2si ecx,QWORD PTR [rcx]

# Tests for op xmm/mem64, regq
	cvtsd2si rcx,xmm4
	cvtsd2si rcx,QWORD PTR [rcx]
	cvttsd2si rcx,xmm4
	cvttsd2si rcx,QWORD PTR [rcx]

# Tests for op regq/mem64, xmm[, xmm]
	cvtsi2sdq xmm4,rcx
	cvtsi2sdq xmm4,QWORD PTR [rcx]
	cvtsi2ssq xmm4,rcx
	cvtsi2ssq xmm4,QWORD PTR [rcx]

# Tests for op imm8, regq/mem64, xmm[, xmm]
	pinsrq xmm4,rcx,100
	pinsrq xmm4,QWORD PTR [rcx],100

# Testsf for op imm8, xmm, regq/mem64
	pextrq rcx,xmm4,100
	pextrq QWORD PTR [rcx],xmm4,100

# Tests for op mem64, xmm[, xmm]
	movlpd xmm4,QWORD PTR [rcx]
	movlps xmm4,QWORD PTR [rcx]
	movhpd xmm4,QWORD PTR [rcx]
	movhps xmm4,QWORD PTR [rcx]

# Tests for op imm8, xmm/mem64, xmm[, xmm]
	cmpsd xmm6,xmm4,100
	cmpsd xmm6,QWORD PTR [rcx],100
	roundsd xmm6,xmm4,100
	roundsd xmm6,QWORD PTR [rcx],100

# Tests for op xmm/mem64, xmm[, xmm]
	addsd xmm6,xmm4
	addsd xmm6,QWORD PTR [rcx]
	cvtsd2ss xmm6,xmm4
	cvtsd2ss xmm6,QWORD PTR [rcx]
	divsd xmm6,xmm4
	divsd xmm6,QWORD PTR [rcx]
	maxsd xmm6,xmm4
	maxsd xmm6,QWORD PTR [rcx]
	minsd xmm6,xmm4
	minsd xmm6,QWORD PTR [rcx]
	mulsd xmm6,xmm4
	mulsd xmm6,QWORD PTR [rcx]
	sqrtsd xmm6,xmm4
	sqrtsd xmm6,QWORD PTR [rcx]
	subsd xmm6,xmm4
	subsd xmm6,QWORD PTR [rcx]
	cmpeqsd xmm6,xmm4
	cmpeqsd xmm6,QWORD PTR [rcx]
	cmpltsd xmm6,xmm4
	cmpltsd xmm6,QWORD PTR [rcx]
	cmplesd xmm6,xmm4
	cmplesd xmm6,QWORD PTR [rcx]
	cmpunordsd xmm6,xmm4
	cmpunordsd xmm6,QWORD PTR [rcx]
	cmpneqsd xmm6,xmm4
	cmpneqsd xmm6,QWORD PTR [rcx]
	cmpnltsd xmm6,xmm4
	cmpnltsd xmm6,QWORD PTR [rcx]
	cmpnlesd xmm6,xmm4
	cmpnlesd xmm6,QWORD PTR [rcx]
	cmpordsd xmm6,xmm4
	cmpordsd xmm6,QWORD PTR [rcx]

# Tests for op xmm/mem32, xmm[, xmm]
	addss xmm6,xmm4
	addss xmm6,DWORD PTR [rcx]
	cvtss2sd xmm6,xmm4
	cvtss2sd xmm6,DWORD PTR [rcx]
	divss xmm6,xmm4
	divss xmm6,DWORD PTR [rcx]
	maxss xmm6,xmm4
	maxss xmm6,DWORD PTR [rcx]
	minss xmm6,xmm4
	minss xmm6,DWORD PTR [rcx]
	mulss xmm6,xmm4
	mulss xmm6,DWORD PTR [rcx]
	rcpss xmm6,xmm4
	rcpss xmm6,DWORD PTR [rcx]
	rsqrtss xmm6,xmm4
	rsqrtss xmm6,DWORD PTR [rcx]
	sqrtss xmm6,xmm4
	sqrtss xmm6,DWORD PTR [rcx]
	subss xmm6,xmm4
	subss xmm6,DWORD PTR [rcx]
	cmpeqss xmm6,xmm4
	cmpeqss xmm6,DWORD PTR [rcx]
	cmpltss xmm6,xmm4
	cmpltss xmm6,DWORD PTR [rcx]
	cmpless xmm6,xmm4
	cmpless xmm6,DWORD PTR [rcx]
	cmpunordss xmm6,xmm4
	cmpunordss xmm6,DWORD PTR [rcx]
	cmpneqss xmm6,xmm4
	cmpneqss xmm6,DWORD PTR [rcx]
	cmpnltss xmm6,xmm4
	cmpnltss xmm6,DWORD PTR [rcx]
	cmpnless xmm6,xmm4
	cmpnless xmm6,DWORD PTR [rcx]
	cmpordss xmm6,xmm4
	cmpordss xmm6,DWORD PTR [rcx]

# Tests for op xmm/mem32, xmm
	comiss xmm6,xmm4
	comiss xmm4,DWORD PTR [rcx]
	pmovsxbd xmm6,xmm4
	pmovsxbd xmm4,DWORD PTR [rcx]
	pmovsxwq xmm6,xmm4
	pmovsxwq xmm4,DWORD PTR [rcx]
	pmovzxbd xmm6,xmm4
	pmovzxbd xmm4,DWORD PTR [rcx]
	pmovzxwq xmm6,xmm4
	pmovzxwq xmm4,DWORD PTR [rcx]
	ucomiss xmm6,xmm4
	ucomiss xmm4,DWORD PTR [rcx]

# Tests for op mem32, xmm
	movss xmm4,DWORD PTR [rcx]

# Tests for op xmm, mem32
	movss DWORD PTR [rcx],xmm4

# Tests for op xmm, regl/mem32
# Tests for op regl/mem32, xmm
	movd ecx,xmm4
	movd DWORD PTR [rcx],xmm4
	movd xmm4,ecx
	movd xmm4,DWORD PTR [rcx]

# Tests for op xmm/mem32, regl
	cvtss2si ecx,xmm4
	cvtss2si ecx,DWORD PTR [rcx]
	cvttss2si ecx,xmm4
	cvttss2si ecx,DWORD PTR [rcx]

# Tests for op xmm/mem32, regq
	cvtss2si rcx,xmm4
	cvtss2si rcx,DWORD PTR [rcx]
	cvttss2si rcx,xmm4
	cvttss2si rcx,DWORD PTR [rcx]

# Tests for op xmm, regq
	movmskpd rcx,xmm4
	movmskps rcx,xmm4
	pmovmskb rcx,xmm4

# Tests for op imm8, xmm, regq/mem32
	extractps rcx,xmm4,100
	extractps DWORD PTR [rcx],xmm4,100
# Tests for op imm8, xmm, regl/mem32
	pextrd ecx,xmm4,100
	pextrd DWORD PTR [rcx],xmm4,100
	extractps ecx,xmm4,100
	extractps DWORD PTR [rcx],xmm4,100

# Tests for op regl/mem32, xmm[, xmm]
	cvtsi2sd xmm4,ecx
	cvtsi2sd xmm4,DWORD PTR [rcx]
	cvtsi2ss xmm4,ecx
	cvtsi2ss xmm4,DWORD PTR [rcx]

# Tests for op imm8, xmm/mem32, xmm[, xmm]
	cmpss xmm6,xmm4,100
	cmpss xmm6,DWORD PTR [rcx],100
	insertps xmm6,xmm4,100
	insertps xmm6,DWORD PTR [rcx],100
	roundss xmm6,xmm4,100
	roundss xmm6,DWORD PTR [rcx],100

# Tests for op xmm/m16, xmm
	pmovsxbq xmm6,xmm4
	pmovsxbq xmm4,WORD PTR [rcx]
	pmovzxbq xmm6,xmm4
	pmovzxbq xmm4,WORD PTR [rcx]

# Tests for op imm8, xmm, regl/mem16
	pextrw ecx,xmm4,100
	pextrw WORD PTR [rcx],xmm4,100

# Tests for op imm8, xmm, regq/mem16
	pextrw rcx,xmm4,100
	pextrw WORD PTR [rcx],xmm4,100

# Tests for op imm8, regl/mem16, xmm[, xmm]
	pinsrw xmm4,ecx,100
	pinsrw xmm4,WORD PTR [rcx],100


	pinsrw xmm4,rcx,100
	pinsrw xmm4,WORD PTR [rcx],100

# Tests for op imm8, xmm, regl/mem8
	pextrb ecx,xmm4,100
	pextrb BYTE PTR [rcx],xmm4,100

# Tests for op imm8, regl/mem8, xmm[, xmm]
	pinsrb xmm4,ecx,100
	pinsrb xmm4,BYTE PTR [rcx],100

# Tests for op imm8, xmm, regq
	pextrw rcx,xmm4,100
# Tests for op imm8, xmm, regq/mem8
	pextrb rcx,xmm4,100
	pextrb BYTE PTR [rcx],xmm4,100

# Tests for op imm8, regl/mem8, xmm[, xmm]
	pinsrb xmm4,ecx,100
	pinsrb xmm4,BYTE PTR [rcx],100

# Tests for op xmm, xmm
	maskmovdqu xmm6,xmm4
	movq xmm6,xmm4

# Tests for op xmm, regl
	movmskpd ecx,xmm4
	movmskps ecx,xmm4
	pmovmskb ecx,xmm4
# Tests for op xmm, xmm[, xmm]
	movhlps xmm6,xmm4
	movlhps xmm6,xmm4
	movsd xmm6,xmm4
	movss xmm6,xmm4

# Tests for op imm8, xmm[, xmm]
	pslld xmm4,100
	pslldq xmm4,100
	psllq xmm4,100
	psllw xmm4,100
	psrad xmm4,100
	psraw xmm4,100
	psrld xmm4,100
	psrldq xmm4,100
	psrlq xmm4,100
	psrlw xmm4,100

# Tests for op imm8, xmm, regl
	pextrw ecx,xmm4,100
