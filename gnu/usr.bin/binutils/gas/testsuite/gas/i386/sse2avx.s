# Check SSE to AVX instructions

	.allow_index_reg
	.text
_start:
# Tests for op mem64
	ldmxcsr (%ecx)
	stmxcsr (%ecx)

# These should not be converted
	data16 ldmxcsr (%ecx)
	data16 stmxcsr (%ecx)

# Tests for op xmm/mem128, xmm
	cvtdq2ps %xmm4,%xmm6
	cvtdq2ps (%ecx),%xmm4
	cvtpd2dq %xmm4,%xmm6
	cvtpd2dq (%ecx),%xmm4
	cvtpd2ps %xmm4,%xmm6
	cvtpd2ps (%ecx),%xmm4
	cvtps2dq %xmm4,%xmm6
	cvtps2dq (%ecx),%xmm4
	cvttpd2dq %xmm4,%xmm6
	cvttpd2dq (%ecx),%xmm4
	cvttps2dq %xmm4,%xmm6
	cvttps2dq (%ecx),%xmm4
	movapd %xmm4,%xmm6
	movapd (%ecx),%xmm4
	movaps %xmm4,%xmm6
	movaps (%ecx),%xmm4
	movdqa %xmm4,%xmm6
	movdqa (%ecx),%xmm4
	movdqu %xmm4,%xmm6
	movdqu (%ecx),%xmm4
	movshdup %xmm4,%xmm6
	movshdup (%ecx),%xmm4
	movsldup %xmm4,%xmm6
	movsldup (%ecx),%xmm4
	movupd %xmm4,%xmm6
	movupd (%ecx),%xmm4
	movups %xmm4,%xmm6
	movups (%ecx),%xmm4
	pabsb %xmm4,%xmm6
	pabsb (%ecx),%xmm4
	pabsw %xmm4,%xmm6
	pabsw (%ecx),%xmm4
	pabsd %xmm4,%xmm6
	pabsd (%ecx),%xmm4
	phminposuw %xmm4,%xmm6
	phminposuw (%ecx),%xmm4
	ptest %xmm4,%xmm6
	ptest (%ecx),%xmm4
	rcpps %xmm4,%xmm6
	rcpps (%ecx),%xmm4
	rsqrtps %xmm4,%xmm6
	rsqrtps (%ecx),%xmm4
	sqrtpd %xmm4,%xmm6
	sqrtpd (%ecx),%xmm4
	sqrtps %xmm4,%xmm6
	sqrtps (%ecx),%xmm4
	aesimc %xmm4,%xmm6
	aesimc (%ecx),%xmm4

# Tests for op xmm, xmm/mem128
	movapd %xmm4,%xmm6
	movapd %xmm4,(%ecx)
	movaps %xmm4,%xmm6
	movaps %xmm4,(%ecx)
	movdqa %xmm4,%xmm6
	movdqa %xmm4,(%ecx)
	movdqu %xmm4,%xmm6
	movdqu %xmm4,(%ecx)
	movupd %xmm4,%xmm6
	movupd %xmm4,(%ecx)
	movups %xmm4,%xmm6
	movups %xmm4,(%ecx)

# Tests for op mem128, xmm
	lddqu (%ecx),%xmm4
	movntdqa (%ecx),%xmm4

# Tests for op xmm, mem128
	movntdq %xmm4,(%ecx)
	movntpd %xmm4,(%ecx)
	movntps %xmm4,(%ecx)

# Tests for op xmm/mem128, xmm[, xmm]
	addpd %xmm4,%xmm6
	addpd (%ecx),%xmm6
	addps %xmm4,%xmm6
	addps (%ecx),%xmm6
	addsubpd %xmm4,%xmm6
	addsubpd (%ecx),%xmm6
	addsubps %xmm4,%xmm6
	addsubps (%ecx),%xmm6
	andnpd %xmm4,%xmm6
	andnpd (%ecx),%xmm6
	andnps %xmm4,%xmm6
	andnps (%ecx),%xmm6
	andpd %xmm4,%xmm6
	andpd (%ecx),%xmm6
	andps %xmm4,%xmm6
	andps (%ecx),%xmm6
	divpd %xmm4,%xmm6
	divpd (%ecx),%xmm6
	divps %xmm4,%xmm6
	divps (%ecx),%xmm6
	haddpd %xmm4,%xmm6
	haddpd (%ecx),%xmm6
	haddps %xmm4,%xmm6
	haddps (%ecx),%xmm6
	hsubpd %xmm4,%xmm6
	hsubpd (%ecx),%xmm6
	hsubps %xmm4,%xmm6
	hsubps (%ecx),%xmm6
	maxpd %xmm4,%xmm6
	maxpd (%ecx),%xmm6
	maxps %xmm4,%xmm6
	maxps (%ecx),%xmm6
	minpd %xmm4,%xmm6
	minpd (%ecx),%xmm6
	minps %xmm4,%xmm6
	minps (%ecx),%xmm6
	mulpd %xmm4,%xmm6
	mulpd (%ecx),%xmm6
	mulps %xmm4,%xmm6
	mulps (%ecx),%xmm6
	orpd %xmm4,%xmm6
	orpd (%ecx),%xmm6
	orps %xmm4,%xmm6
	orps (%ecx),%xmm6
	packsswb %xmm4,%xmm6
	packsswb (%ecx),%xmm6
	packssdw %xmm4,%xmm6
	packssdw (%ecx),%xmm6
	packuswb %xmm4,%xmm6
	packuswb (%ecx),%xmm6
	packusdw %xmm4,%xmm6
	packusdw (%ecx),%xmm6
	paddb %xmm4,%xmm6
	paddb (%ecx),%xmm6
	paddw %xmm4,%xmm6
	paddw (%ecx),%xmm6
	paddd %xmm4,%xmm6
	paddd (%ecx),%xmm6
	paddq %xmm4,%xmm6
	paddq (%ecx),%xmm6
	paddsb %xmm4,%xmm6
	paddsb (%ecx),%xmm6
	paddsw %xmm4,%xmm6
	paddsw (%ecx),%xmm6
	paddusb %xmm4,%xmm6
	paddusb (%ecx),%xmm6
	paddusw %xmm4,%xmm6
	paddusw (%ecx),%xmm6
	pand %xmm4,%xmm6
	pand (%ecx),%xmm6
	pandn %xmm4,%xmm6
	pandn (%ecx),%xmm6
	pavgb %xmm4,%xmm6
	pavgb (%ecx),%xmm6
	pavgw %xmm4,%xmm6
	pavgw (%ecx),%xmm6
	pclmullqlqdq %xmm4,%xmm6
	pclmullqlqdq (%ecx),%xmm6
	pclmulhqlqdq %xmm4,%xmm6
	pclmulhqlqdq (%ecx),%xmm6
	pclmullqhqdq %xmm4,%xmm6
	pclmullqhqdq (%ecx),%xmm6
	pclmulhqhqdq %xmm4,%xmm6
	pclmulhqhqdq (%ecx),%xmm6
	pcmpeqb %xmm4,%xmm6
	pcmpeqb (%ecx),%xmm6
	pcmpeqw %xmm4,%xmm6
	pcmpeqw (%ecx),%xmm6
	pcmpeqd %xmm4,%xmm6
	pcmpeqd (%ecx),%xmm6
	pcmpeqq %xmm4,%xmm6
	pcmpeqq (%ecx),%xmm6
	pcmpgtb %xmm4,%xmm6
	pcmpgtb (%ecx),%xmm6
	pcmpgtw %xmm4,%xmm6
	pcmpgtw (%ecx),%xmm6
	pcmpgtd %xmm4,%xmm6
	pcmpgtd (%ecx),%xmm6
	pcmpgtq %xmm4,%xmm6
	pcmpgtq (%ecx),%xmm6
	phaddw %xmm4,%xmm6
	phaddw (%ecx),%xmm6
	phaddd %xmm4,%xmm6
	phaddd (%ecx),%xmm6
	phaddsw %xmm4,%xmm6
	phaddsw (%ecx),%xmm6
	phsubw %xmm4,%xmm6
	phsubw (%ecx),%xmm6
	phsubd %xmm4,%xmm6
	phsubd (%ecx),%xmm6
	phsubsw %xmm4,%xmm6
	phsubsw (%ecx),%xmm6
	pmaddwd %xmm4,%xmm6
	pmaddwd (%ecx),%xmm6
	pmaddubsw %xmm4,%xmm6
	pmaddubsw (%ecx),%xmm6
	pmaxsb %xmm4,%xmm6
	pmaxsb (%ecx),%xmm6
	pmaxsw %xmm4,%xmm6
	pmaxsw (%ecx),%xmm6
	pmaxsd %xmm4,%xmm6
	pmaxsd (%ecx),%xmm6
	pmaxub %xmm4,%xmm6
	pmaxub (%ecx),%xmm6
	pmaxuw %xmm4,%xmm6
	pmaxuw (%ecx),%xmm6
	pmaxud %xmm4,%xmm6
	pmaxud (%ecx),%xmm6
	pminsb %xmm4,%xmm6
	pminsb (%ecx),%xmm6
	pminsw %xmm4,%xmm6
	pminsw (%ecx),%xmm6
	pminsd %xmm4,%xmm6
	pminsd (%ecx),%xmm6
	pminub %xmm4,%xmm6
	pminub (%ecx),%xmm6
	pminuw %xmm4,%xmm6
	pminuw (%ecx),%xmm6
	pminud %xmm4,%xmm6
	pminud (%ecx),%xmm6
	pmulhuw %xmm4,%xmm6
	pmulhuw (%ecx),%xmm6
	pmulhrsw %xmm4,%xmm6
	pmulhrsw (%ecx),%xmm6
	pmulhw %xmm4,%xmm6
	pmulhw (%ecx),%xmm6
	pmullw %xmm4,%xmm6
	pmullw (%ecx),%xmm6
	pmulld %xmm4,%xmm6
	pmulld (%ecx),%xmm6
	pmuludq %xmm4,%xmm6
	pmuludq (%ecx),%xmm6
	pmuldq %xmm4,%xmm6
	pmuldq (%ecx),%xmm6
	por %xmm4,%xmm6
	por (%ecx),%xmm6
	psadbw %xmm4,%xmm6
	psadbw (%ecx),%xmm6
	pshufb %xmm4,%xmm6
	pshufb (%ecx),%xmm6
	psignb %xmm4,%xmm6
	psignb (%ecx),%xmm6
	psignw %xmm4,%xmm6
	psignw (%ecx),%xmm6
	psignd %xmm4,%xmm6
	psignd (%ecx),%xmm6
	psllw %xmm4,%xmm6
	psllw (%ecx),%xmm6
	pslld %xmm4,%xmm6
	pslld (%ecx),%xmm6
	psllq %xmm4,%xmm6
	psllq (%ecx),%xmm6
	psraw %xmm4,%xmm6
	psraw (%ecx),%xmm6
	psrad %xmm4,%xmm6
	psrad (%ecx),%xmm6
	psrlw %xmm4,%xmm6
	psrlw (%ecx),%xmm6
	psrld %xmm4,%xmm6
	psrld (%ecx),%xmm6
	psrlq %xmm4,%xmm6
	psrlq (%ecx),%xmm6
	psubb %xmm4,%xmm6
	psubb (%ecx),%xmm6
	psubw %xmm4,%xmm6
	psubw (%ecx),%xmm6
	psubd %xmm4,%xmm6
	psubd (%ecx),%xmm6
	psubq %xmm4,%xmm6
	psubq (%ecx),%xmm6
	psubsb %xmm4,%xmm6
	psubsb (%ecx),%xmm6
	psubsw %xmm4,%xmm6
	psubsw (%ecx),%xmm6
	psubusb %xmm4,%xmm6
	psubusb (%ecx),%xmm6
	psubusw %xmm4,%xmm6
	psubusw (%ecx),%xmm6
	punpckhbw %xmm4,%xmm6
	punpckhbw (%ecx),%xmm6
	punpckhwd %xmm4,%xmm6
	punpckhwd (%ecx),%xmm6
	punpckhdq %xmm4,%xmm6
	punpckhdq (%ecx),%xmm6
	punpckhqdq %xmm4,%xmm6
	punpckhqdq (%ecx),%xmm6
	punpcklbw %xmm4,%xmm6
	punpcklbw (%ecx),%xmm6
	punpcklwd %xmm4,%xmm6
	punpcklwd (%ecx),%xmm6
	punpckldq %xmm4,%xmm6
	punpckldq (%ecx),%xmm6
	punpcklqdq %xmm4,%xmm6
	punpcklqdq (%ecx),%xmm6
	pxor %xmm4,%xmm6
	pxor (%ecx),%xmm6
	subpd %xmm4,%xmm6
	subpd (%ecx),%xmm6
	subps %xmm4,%xmm6
	subps (%ecx),%xmm6
	unpckhpd %xmm4,%xmm6
	unpckhpd (%ecx),%xmm6
	unpckhps %xmm4,%xmm6
	unpckhps (%ecx),%xmm6
	unpcklpd %xmm4,%xmm6
	unpcklpd (%ecx),%xmm6
	unpcklps %xmm4,%xmm6
	unpcklps (%ecx),%xmm6
	xorpd %xmm4,%xmm6
	xorpd (%ecx),%xmm6
	xorps %xmm4,%xmm6
	xorps (%ecx),%xmm6
	aesenc %xmm4,%xmm6
	aesenc (%ecx),%xmm6
	aesenclast %xmm4,%xmm6
	aesenclast (%ecx),%xmm6
	aesdec %xmm4,%xmm6
	aesdec (%ecx),%xmm6
	aesdeclast %xmm4,%xmm6
	aesdeclast (%ecx),%xmm6
	cmpeqpd %xmm4,%xmm6
	cmpeqpd (%ecx),%xmm6
	cmpeqps %xmm4,%xmm6
	cmpeqps (%ecx),%xmm6
	cmpltpd %xmm4,%xmm6
	cmpltpd (%ecx),%xmm6
	cmpltps %xmm4,%xmm6
	cmpltps (%ecx),%xmm6
	cmplepd %xmm4,%xmm6
	cmplepd (%ecx),%xmm6
	cmpleps %xmm4,%xmm6
	cmpleps (%ecx),%xmm6
	cmpunordpd %xmm4,%xmm6
	cmpunordpd (%ecx),%xmm6
	cmpunordps %xmm4,%xmm6
	cmpunordps (%ecx),%xmm6
	cmpneqpd %xmm4,%xmm6
	cmpneqpd (%ecx),%xmm6
	cmpneqps %xmm4,%xmm6
	cmpneqps (%ecx),%xmm6
	cmpnltpd %xmm4,%xmm6
	cmpnltpd (%ecx),%xmm6
	cmpnltps %xmm4,%xmm6
	cmpnltps (%ecx),%xmm6
	cmpnlepd %xmm4,%xmm6
	cmpnlepd (%ecx),%xmm6
	cmpnleps %xmm4,%xmm6
	cmpnleps (%ecx),%xmm6
	cmpordpd %xmm4,%xmm6
	cmpordpd (%ecx),%xmm6
	cmpordps %xmm4,%xmm6
	cmpordps (%ecx),%xmm6

# Tests for op imm8, xmm/mem128, xmm
	aeskeygenassist $100,%xmm4,%xmm6
	aeskeygenassist $100,(%ecx),%xmm6
	pcmpestri $100,%xmm4,%xmm6
	pcmpestri $100,(%ecx),%xmm6
	pcmpestrm $100,%xmm4,%xmm6
	pcmpestrm $100,(%ecx),%xmm6
	pcmpistri $100,%xmm4,%xmm6
	pcmpistri $100,(%ecx),%xmm6
	pcmpistrm $100,%xmm4,%xmm6
	pcmpistrm $100,(%ecx),%xmm6
	pshufd $100,%xmm4,%xmm6
	pshufd $100,(%ecx),%xmm6
	pshufhw $100,%xmm4,%xmm6
	pshufhw $100,(%ecx),%xmm6
	pshuflw $100,%xmm4,%xmm6
	pshuflw $100,(%ecx),%xmm6
	roundpd $100,%xmm4,%xmm6
	roundpd $100,(%ecx),%xmm6
	roundps $100,%xmm4,%xmm6
	roundps $100,(%ecx),%xmm6

# Tests for op imm8, xmm/mem128, xmm[, xmm]
	blendpd $100,%xmm4,%xmm6
	blendpd $100,(%ecx),%xmm6
	blendps $100,%xmm4,%xmm6
	blendps $100,(%ecx),%xmm6
	cmppd $100,%xmm4,%xmm6
	cmppd $100,(%ecx),%xmm6
	cmpps $100,%xmm4,%xmm6
	cmpps $100,(%ecx),%xmm6
	dppd $100,%xmm4,%xmm6
	dppd $100,(%ecx),%xmm6
	dpps $100,%xmm4,%xmm6
	dpps $100,(%ecx),%xmm6
	mpsadbw $100,%xmm4,%xmm6
	mpsadbw $100,(%ecx),%xmm6
	palignr $100,%xmm4,%xmm6
	palignr $100,(%ecx),%xmm6
	pblendw $100,%xmm4,%xmm6
	pblendw $100,(%ecx),%xmm6
	pclmulqdq $100,%xmm4,%xmm6
	pclmulqdq $100,(%ecx),%xmm6
	shufpd $100,%xmm4,%xmm6
	shufpd $100,(%ecx),%xmm6
	shufps $100,%xmm4,%xmm6
	shufps $100,(%ecx),%xmm6

# Tests for op xmm0, xmm/mem128, xmm[, xmm]
	blendvpd %xmm0,%xmm4,%xmm6
	blendvpd %xmm0,(%ecx),%xmm6
	blendvpd %xmm4,%xmm6
	blendvpd (%ecx),%xmm6
	blendvps %xmm0,%xmm4,%xmm6
	blendvps %xmm0,(%ecx),%xmm6
	blendvps %xmm4,%xmm6
	blendvps (%ecx),%xmm6
	pblendvb %xmm0,%xmm4,%xmm6
	pblendvb %xmm0,(%ecx),%xmm6
	pblendvb %xmm4,%xmm6
	pblendvb (%ecx),%xmm6

# Tests for op xmm/mem64, xmm
	comisd %xmm4,%xmm6
	comisd (%ecx),%xmm4
	cvtdq2pd %xmm4,%xmm6
	cvtdq2pd (%ecx),%xmm4
	cvtpi2pd (%ecx),%xmm4
	cvtps2pd %xmm4,%xmm6
	cvtps2pd (%ecx),%xmm4
	movddup %xmm4,%xmm6
	movddup (%ecx),%xmm4
	pmovsxbw %xmm4,%xmm6
	pmovsxbw (%ecx),%xmm4
	pmovsxwd %xmm4,%xmm6
	pmovsxwd (%ecx),%xmm4
	pmovsxdq %xmm4,%xmm6
	pmovsxdq (%ecx),%xmm4
	pmovzxbw %xmm4,%xmm6
	pmovzxbw (%ecx),%xmm4
	pmovzxwd %xmm4,%xmm6
	pmovzxwd (%ecx),%xmm4
	pmovzxdq %xmm4,%xmm6
	pmovzxdq (%ecx),%xmm4
	ucomisd %xmm4,%xmm6
	ucomisd (%ecx),%xmm4

# Tests for op mem64, xmm
	movsd (%ecx),%xmm4

# Tests for op xmm, mem64
	movlpd %xmm4,(%ecx)
	movlps %xmm4,(%ecx)
	movhpd %xmm4,(%ecx)
	movhps %xmm4,(%ecx)
	movsd %xmm4,(%ecx)

# Tests for op xmm, regq/mem64
# Tests for op regq/mem64, xmm
	movq %xmm4,(%ecx)
	movq (%ecx),%xmm4

# Tests for op xmm/mem64, regl
	cvtsd2si %xmm4,%ecx
	cvtsd2si (%ecx),%ecx
	cvttsd2si %xmm4,%ecx
	cvttsd2si (%ecx),%ecx

# Tests for op mem64, xmm[, xmm]
	movlpd (%ecx),%xmm4
	movlps (%ecx),%xmm4
	movhpd (%ecx),%xmm4
	movhps (%ecx),%xmm4

# Tests for op imm8, xmm/mem64, xmm[, xmm]
	cmpsd $100,%xmm4,%xmm6
	cmpsd $100,(%ecx),%xmm6
	roundsd $100,%xmm4,%xmm6
	roundsd $100,(%ecx),%xmm6

# Tests for op xmm/mem64, xmm[, xmm]
	addsd %xmm4,%xmm6
	addsd (%ecx),%xmm6
	cvtsd2ss %xmm4,%xmm6
	cvtsd2ss (%ecx),%xmm6
	divsd %xmm4,%xmm6
	divsd (%ecx),%xmm6
	maxsd %xmm4,%xmm6
	maxsd (%ecx),%xmm6
	minsd %xmm4,%xmm6
	minsd (%ecx),%xmm6
	mulsd %xmm4,%xmm6
	mulsd (%ecx),%xmm6
	sqrtsd %xmm4,%xmm6
	sqrtsd (%ecx),%xmm6
	subsd %xmm4,%xmm6
	subsd (%ecx),%xmm6
	cmpeqsd %xmm4,%xmm6
	cmpeqsd (%ecx),%xmm6
	cmpltsd %xmm4,%xmm6
	cmpltsd (%ecx),%xmm6
	cmplesd %xmm4,%xmm6
	cmplesd (%ecx),%xmm6
	cmpunordsd %xmm4,%xmm6
	cmpunordsd (%ecx),%xmm6
	cmpneqsd %xmm4,%xmm6
	cmpneqsd (%ecx),%xmm6
	cmpnltsd %xmm4,%xmm6
	cmpnltsd (%ecx),%xmm6
	cmpnlesd %xmm4,%xmm6
	cmpnlesd (%ecx),%xmm6
	cmpordsd %xmm4,%xmm6
	cmpordsd (%ecx),%xmm6

# Tests for op xmm/mem32, xmm[, xmm]
	addss %xmm4,%xmm6
	addss (%ecx),%xmm6
	cvtss2sd %xmm4,%xmm6
	cvtss2sd (%ecx),%xmm6
	divss %xmm4,%xmm6
	divss (%ecx),%xmm6
	maxss %xmm4,%xmm6
	maxss (%ecx),%xmm6
	minss %xmm4,%xmm6
	minss (%ecx),%xmm6
	mulss %xmm4,%xmm6
	mulss (%ecx),%xmm6
	rcpss %xmm4,%xmm6
	rcpss (%ecx),%xmm6
	rsqrtss %xmm4,%xmm6
	rsqrtss (%ecx),%xmm6
	sqrtss %xmm4,%xmm6
	sqrtss (%ecx),%xmm6
	subss %xmm4,%xmm6
	subss (%ecx),%xmm6
	cmpeqss %xmm4,%xmm6
	cmpeqss (%ecx),%xmm6
	cmpltss %xmm4,%xmm6
	cmpltss (%ecx),%xmm6
	cmpless %xmm4,%xmm6
	cmpless (%ecx),%xmm6
	cmpunordss %xmm4,%xmm6
	cmpunordss (%ecx),%xmm6
	cmpneqss %xmm4,%xmm6
	cmpneqss (%ecx),%xmm6
	cmpnltss %xmm4,%xmm6
	cmpnltss (%ecx),%xmm6
	cmpnless %xmm4,%xmm6
	cmpnless (%ecx),%xmm6
	cmpordss %xmm4,%xmm6
	cmpordss (%ecx),%xmm6

# Tests for op xmm/mem32, xmm
	comiss %xmm4,%xmm6
	comiss (%ecx),%xmm4
	pmovsxbd %xmm4,%xmm6
	pmovsxbd (%ecx),%xmm4
	pmovsxwq %xmm4,%xmm6
	pmovsxwq (%ecx),%xmm4
	pmovzxbd %xmm4,%xmm6
	pmovzxbd (%ecx),%xmm4
	pmovzxwq %xmm4,%xmm6
	pmovzxwq (%ecx),%xmm4
	ucomiss %xmm4,%xmm6
	ucomiss (%ecx),%xmm4

# Tests for op mem32, xmm
	movss (%ecx),%xmm4

# Tests for op xmm, mem32
	movss %xmm4,(%ecx)

# Tests for op xmm, regl/mem32
# Tests for op regl/mem32, xmm
	movd %xmm4,%ecx
	movd %xmm4,(%ecx)
	movd %ecx,%xmm4
	movd (%ecx),%xmm4

# Tests for op xmm/mem32, regl
	cvtss2si %xmm4,%ecx
	cvtss2si (%ecx),%ecx
	cvttss2si %xmm4,%ecx
	cvttss2si (%ecx),%ecx

# Tests for op imm8, xmm, regq/mem32
	extractps $100,%xmm4,(%ecx)
# Tests for op imm8, xmm, regl/mem32
	pextrd $100,%xmm4,%ecx
	pextrd $100,%xmm4,(%ecx)
	extractps $100,%xmm4,%ecx
	extractps $100,%xmm4,(%ecx)

# Tests for op regl/mem32, xmm[, xmm]
	cvtsi2sd %ecx,%xmm4
	cvtsi2sd (%ecx),%xmm4
	cvtsi2ss %ecx,%xmm4
	cvtsi2ss (%ecx),%xmm4

# Tests for op imm8, xmm/mem32, xmm[, xmm]
	cmpss $100,%xmm4,%xmm6
	cmpss $100,(%ecx),%xmm6
	insertps $100,%xmm4,%xmm6
	insertps $100,(%ecx),%xmm6
	roundss $100,%xmm4,%xmm6
	roundss $100,(%ecx),%xmm6

# Tests for op xmm/m16, xmm
	pmovsxbq %xmm4,%xmm6
	pmovsxbq (%ecx),%xmm4
	pmovzxbq %xmm4,%xmm6
	pmovzxbq (%ecx),%xmm4

# Tests for op imm8, xmm, regl/mem16
	pextrw $100,%xmm4,%ecx
	pextrw $100,%xmm4,(%ecx)

# Tests for op imm8, xmm, regq/mem16
	pextrw $100,%xmm4,(%ecx)

# Tests for op imm8, regl/mem16, xmm[, xmm]
	pinsrw $100,%ecx,%xmm4
	pinsrw $100,(%ecx),%xmm4


# Tests for op imm8, xmm, regl/mem8
	pextrb $100,%xmm4,%ecx
	pextrb $100,%xmm4,(%ecx)

# Tests for op imm8, regl/mem8, xmm[, xmm]
	pinsrb $100,%ecx,%xmm4
	pinsrb $100,(%ecx),%xmm4

# Tests for op imm8, xmm, regq/mem8
	pextrb $100,%xmm4,(%ecx)

# Tests for op imm8, regl/mem8, xmm[, xmm]
	pinsrb $100,%ecx,%xmm4
	pinsrb $100,(%ecx),%xmm4

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


	.intel_syntax noprefix
# Tests for op mem64
	ldmxcsr DWORD PTR [ecx]
	stmxcsr DWORD PTR [ecx]

# Tests for op xmm/mem128, xmm
	cvtdq2ps xmm6,xmm4
	cvtdq2ps xmm4,XMMWORD PTR [ecx]
	cvtpd2dq xmm6,xmm4
	cvtpd2dq xmm4,XMMWORD PTR [ecx]
	cvtpd2ps xmm6,xmm4
	cvtpd2ps xmm4,XMMWORD PTR [ecx]
	cvtps2dq xmm6,xmm4
	cvtps2dq xmm4,XMMWORD PTR [ecx]
	cvttpd2dq xmm6,xmm4
	cvttpd2dq xmm4,XMMWORD PTR [ecx]
	cvttps2dq xmm6,xmm4
	cvttps2dq xmm4,XMMWORD PTR [ecx]
	movapd xmm6,xmm4
	movapd xmm4,XMMWORD PTR [ecx]
	movaps xmm6,xmm4
	movaps xmm4,XMMWORD PTR [ecx]
	movdqa xmm6,xmm4
	movdqa xmm4,XMMWORD PTR [ecx]
	movdqu xmm6,xmm4
	movdqu xmm4,XMMWORD PTR [ecx]
	movshdup xmm6,xmm4
	movshdup xmm4,XMMWORD PTR [ecx]
	movsldup xmm6,xmm4
	movsldup xmm4,XMMWORD PTR [ecx]
	movupd xmm6,xmm4
	movupd xmm4,XMMWORD PTR [ecx]
	movups xmm6,xmm4
	movups xmm4,XMMWORD PTR [ecx]
	pabsb xmm6,xmm4
	pabsb xmm4,XMMWORD PTR [ecx]
	pabsw xmm6,xmm4
	pabsw xmm4,XMMWORD PTR [ecx]
	pabsd xmm6,xmm4
	pabsd xmm4,XMMWORD PTR [ecx]
	phminposuw xmm6,xmm4
	phminposuw xmm4,XMMWORD PTR [ecx]
	ptest xmm6,xmm4
	ptest xmm4,XMMWORD PTR [ecx]
	rcpps xmm6,xmm4
	rcpps xmm4,XMMWORD PTR [ecx]
	rsqrtps xmm6,xmm4
	rsqrtps xmm4,XMMWORD PTR [ecx]
	sqrtpd xmm6,xmm4
	sqrtpd xmm4,XMMWORD PTR [ecx]
	sqrtps xmm6,xmm4
	sqrtps xmm4,XMMWORD PTR [ecx]
	aesimc xmm6,xmm4
	aesimc xmm4,XMMWORD PTR [ecx]

# Tests for op xmm, xmm/mem128
	movapd xmm6,xmm4
	movapd XMMWORD PTR [ecx],xmm4
	movaps xmm6,xmm4
	movaps XMMWORD PTR [ecx],xmm4
	movdqa xmm6,xmm4
	movdqa XMMWORD PTR [ecx],xmm4
	movdqu xmm6,xmm4
	movdqu XMMWORD PTR [ecx],xmm4
	movupd xmm6,xmm4
	movupd XMMWORD PTR [ecx],xmm4
	movups xmm6,xmm4
	movups XMMWORD PTR [ecx],xmm4

# Tests for op mem128, xmm
	lddqu xmm4,XMMWORD PTR [ecx]
	movntdqa xmm4,XMMWORD PTR [ecx]

# Tests for op xmm, mem128
	movntdq XMMWORD PTR [ecx],xmm4
	movntpd XMMWORD PTR [ecx],xmm4
	movntps XMMWORD PTR [ecx],xmm4

# Tests for op xmm/mem128, xmm[, xmm]
	addpd xmm6,xmm4
	addpd xmm6,XMMWORD PTR [ecx]
	addps xmm6,xmm4
	addps xmm6,XMMWORD PTR [ecx]
	addsubpd xmm6,xmm4
	addsubpd xmm6,XMMWORD PTR [ecx]
	addsubps xmm6,xmm4
	addsubps xmm6,XMMWORD PTR [ecx]
	andnpd xmm6,xmm4
	andnpd xmm6,XMMWORD PTR [ecx]
	andnps xmm6,xmm4
	andnps xmm6,XMMWORD PTR [ecx]
	andpd xmm6,xmm4
	andpd xmm6,XMMWORD PTR [ecx]
	andps xmm6,xmm4
	andps xmm6,XMMWORD PTR [ecx]
	divpd xmm6,xmm4
	divpd xmm6,XMMWORD PTR [ecx]
	divps xmm6,xmm4
	divps xmm6,XMMWORD PTR [ecx]
	haddpd xmm6,xmm4
	haddpd xmm6,XMMWORD PTR [ecx]
	haddps xmm6,xmm4
	haddps xmm6,XMMWORD PTR [ecx]
	hsubpd xmm6,xmm4
	hsubpd xmm6,XMMWORD PTR [ecx]
	hsubps xmm6,xmm4
	hsubps xmm6,XMMWORD PTR [ecx]
	maxpd xmm6,xmm4
	maxpd xmm6,XMMWORD PTR [ecx]
	maxps xmm6,xmm4
	maxps xmm6,XMMWORD PTR [ecx]
	minpd xmm6,xmm4
	minpd xmm6,XMMWORD PTR [ecx]
	minps xmm6,xmm4
	minps xmm6,XMMWORD PTR [ecx]
	mulpd xmm6,xmm4
	mulpd xmm6,XMMWORD PTR [ecx]
	mulps xmm6,xmm4
	mulps xmm6,XMMWORD PTR [ecx]
	orpd xmm6,xmm4
	orpd xmm6,XMMWORD PTR [ecx]
	orps xmm6,xmm4
	orps xmm6,XMMWORD PTR [ecx]
	packsswb xmm6,xmm4
	packsswb xmm6,XMMWORD PTR [ecx]
	packssdw xmm6,xmm4
	packssdw xmm6,XMMWORD PTR [ecx]
	packuswb xmm6,xmm4
	packuswb xmm6,XMMWORD PTR [ecx]
	packusdw xmm6,xmm4
	packusdw xmm6,XMMWORD PTR [ecx]
	paddb xmm6,xmm4
	paddb xmm6,XMMWORD PTR [ecx]
	paddw xmm6,xmm4
	paddw xmm6,XMMWORD PTR [ecx]
	paddd xmm6,xmm4
	paddd xmm6,XMMWORD PTR [ecx]
	paddq xmm6,xmm4
	paddq xmm6,XMMWORD PTR [ecx]
	paddsb xmm6,xmm4
	paddsb xmm6,XMMWORD PTR [ecx]
	paddsw xmm6,xmm4
	paddsw xmm6,XMMWORD PTR [ecx]
	paddusb xmm6,xmm4
	paddusb xmm6,XMMWORD PTR [ecx]
	paddusw xmm6,xmm4
	paddusw xmm6,XMMWORD PTR [ecx]
	pand xmm6,xmm4
	pand xmm6,XMMWORD PTR [ecx]
	pandn xmm6,xmm4
	pandn xmm6,XMMWORD PTR [ecx]
	pavgb xmm6,xmm4
	pavgb xmm6,XMMWORD PTR [ecx]
	pavgw xmm6,xmm4
	pavgw xmm6,XMMWORD PTR [ecx]
	pclmullqlqdq xmm6,xmm4
	pclmullqlqdq xmm6,XMMWORD PTR [ecx]
	pclmulhqlqdq xmm6,xmm4
	pclmulhqlqdq xmm6,XMMWORD PTR [ecx]
	pclmullqhqdq xmm6,xmm4
	pclmullqhqdq xmm6,XMMWORD PTR [ecx]
	pclmulhqhqdq xmm6,xmm4
	pclmulhqhqdq xmm6,XMMWORD PTR [ecx]
	pcmpeqb xmm6,xmm4
	pcmpeqb xmm6,XMMWORD PTR [ecx]
	pcmpeqw xmm6,xmm4
	pcmpeqw xmm6,XMMWORD PTR [ecx]
	pcmpeqd xmm6,xmm4
	pcmpeqd xmm6,XMMWORD PTR [ecx]
	pcmpeqq xmm6,xmm4
	pcmpeqq xmm6,XMMWORD PTR [ecx]
	pcmpgtb xmm6,xmm4
	pcmpgtb xmm6,XMMWORD PTR [ecx]
	pcmpgtw xmm6,xmm4
	pcmpgtw xmm6,XMMWORD PTR [ecx]
	pcmpgtd xmm6,xmm4
	pcmpgtd xmm6,XMMWORD PTR [ecx]
	pcmpgtq xmm6,xmm4
	pcmpgtq xmm6,XMMWORD PTR [ecx]
	phaddw xmm6,xmm4
	phaddw xmm6,XMMWORD PTR [ecx]
	phaddd xmm6,xmm4
	phaddd xmm6,XMMWORD PTR [ecx]
	phaddsw xmm6,xmm4
	phaddsw xmm6,XMMWORD PTR [ecx]
	phsubw xmm6,xmm4
	phsubw xmm6,XMMWORD PTR [ecx]
	phsubd xmm6,xmm4
	phsubd xmm6,XMMWORD PTR [ecx]
	phsubsw xmm6,xmm4
	phsubsw xmm6,XMMWORD PTR [ecx]
	pmaddwd xmm6,xmm4
	pmaddwd xmm6,XMMWORD PTR [ecx]
	pmaddubsw xmm6,xmm4
	pmaddubsw xmm6,XMMWORD PTR [ecx]
	pmaxsb xmm6,xmm4
	pmaxsb xmm6,XMMWORD PTR [ecx]
	pmaxsw xmm6,xmm4
	pmaxsw xmm6,XMMWORD PTR [ecx]
	pmaxsd xmm6,xmm4
	pmaxsd xmm6,XMMWORD PTR [ecx]
	pmaxub xmm6,xmm4
	pmaxub xmm6,XMMWORD PTR [ecx]
	pmaxuw xmm6,xmm4
	pmaxuw xmm6,XMMWORD PTR [ecx]
	pmaxud xmm6,xmm4
	pmaxud xmm6,XMMWORD PTR [ecx]
	pminsb xmm6,xmm4
	pminsb xmm6,XMMWORD PTR [ecx]
	pminsw xmm6,xmm4
	pminsw xmm6,XMMWORD PTR [ecx]
	pminsd xmm6,xmm4
	pminsd xmm6,XMMWORD PTR [ecx]
	pminub xmm6,xmm4
	pminub xmm6,XMMWORD PTR [ecx]
	pminuw xmm6,xmm4
	pminuw xmm6,XMMWORD PTR [ecx]
	pminud xmm6,xmm4
	pminud xmm6,XMMWORD PTR [ecx]
	pmulhuw xmm6,xmm4
	pmulhuw xmm6,XMMWORD PTR [ecx]
	pmulhrsw xmm6,xmm4
	pmulhrsw xmm6,XMMWORD PTR [ecx]
	pmulhw xmm6,xmm4
	pmulhw xmm6,XMMWORD PTR [ecx]
	pmullw xmm6,xmm4
	pmullw xmm6,XMMWORD PTR [ecx]
	pmulld xmm6,xmm4
	pmulld xmm6,XMMWORD PTR [ecx]
	pmuludq xmm6,xmm4
	pmuludq xmm6,XMMWORD PTR [ecx]
	pmuldq xmm6,xmm4
	pmuldq xmm6,XMMWORD PTR [ecx]
	por xmm6,xmm4
	por xmm6,XMMWORD PTR [ecx]
	psadbw xmm6,xmm4
	psadbw xmm6,XMMWORD PTR [ecx]
	pshufb xmm6,xmm4
	pshufb xmm6,XMMWORD PTR [ecx]
	psignb xmm6,xmm4
	psignb xmm6,XMMWORD PTR [ecx]
	psignw xmm6,xmm4
	psignw xmm6,XMMWORD PTR [ecx]
	psignd xmm6,xmm4
	psignd xmm6,XMMWORD PTR [ecx]
	psllw xmm6,xmm4
	psllw xmm6,XMMWORD PTR [ecx]
	pslld xmm6,xmm4
	pslld xmm6,XMMWORD PTR [ecx]
	psllq xmm6,xmm4
	psllq xmm6,XMMWORD PTR [ecx]
	psraw xmm6,xmm4
	psraw xmm6,XMMWORD PTR [ecx]
	psrad xmm6,xmm4
	psrad xmm6,XMMWORD PTR [ecx]
	psrlw xmm6,xmm4
	psrlw xmm6,XMMWORD PTR [ecx]
	psrld xmm6,xmm4
	psrld xmm6,XMMWORD PTR [ecx]
	psrlq xmm6,xmm4
	psrlq xmm6,XMMWORD PTR [ecx]
	psubb xmm6,xmm4
	psubb xmm6,XMMWORD PTR [ecx]
	psubw xmm6,xmm4
	psubw xmm6,XMMWORD PTR [ecx]
	psubd xmm6,xmm4
	psubd xmm6,XMMWORD PTR [ecx]
	psubq xmm6,xmm4
	psubq xmm6,XMMWORD PTR [ecx]
	psubsb xmm6,xmm4
	psubsb xmm6,XMMWORD PTR [ecx]
	psubsw xmm6,xmm4
	psubsw xmm6,XMMWORD PTR [ecx]
	psubusb xmm6,xmm4
	psubusb xmm6,XMMWORD PTR [ecx]
	psubusw xmm6,xmm4
	psubusw xmm6,XMMWORD PTR [ecx]
	punpckhbw xmm6,xmm4
	punpckhbw xmm6,XMMWORD PTR [ecx]
	punpckhwd xmm6,xmm4
	punpckhwd xmm6,XMMWORD PTR [ecx]
	punpckhdq xmm6,xmm4
	punpckhdq xmm6,XMMWORD PTR [ecx]
	punpckhqdq xmm6,xmm4
	punpckhqdq xmm6,XMMWORD PTR [ecx]
	punpcklbw xmm6,xmm4
	punpcklbw xmm6,XMMWORD PTR [ecx]
	punpcklwd xmm6,xmm4
	punpcklwd xmm6,XMMWORD PTR [ecx]
	punpckldq xmm6,xmm4
	punpckldq xmm6,XMMWORD PTR [ecx]
	punpcklqdq xmm6,xmm4
	punpcklqdq xmm6,XMMWORD PTR [ecx]
	pxor xmm6,xmm4
	pxor xmm6,XMMWORD PTR [ecx]
	subpd xmm6,xmm4
	subpd xmm6,XMMWORD PTR [ecx]
	subps xmm6,xmm4
	subps xmm6,XMMWORD PTR [ecx]
	unpckhpd xmm6,xmm4
	unpckhpd xmm6,XMMWORD PTR [ecx]
	unpckhps xmm6,xmm4
	unpckhps xmm6,XMMWORD PTR [ecx]
	unpcklpd xmm6,xmm4
	unpcklpd xmm6,XMMWORD PTR [ecx]
	unpcklps xmm6,xmm4
	unpcklps xmm6,XMMWORD PTR [ecx]
	xorpd xmm6,xmm4
	xorpd xmm6,XMMWORD PTR [ecx]
	xorps xmm6,xmm4
	xorps xmm6,XMMWORD PTR [ecx]
	aesenc xmm6,xmm4
	aesenc xmm6,XMMWORD PTR [ecx]
	aesenclast xmm6,xmm4
	aesenclast xmm6,XMMWORD PTR [ecx]
	aesdec xmm6,xmm4
	aesdec xmm6,XMMWORD PTR [ecx]
	aesdeclast xmm6,xmm4
	aesdeclast xmm6,XMMWORD PTR [ecx]
	cmpeqpd xmm6,xmm4
	cmpeqpd xmm6,XMMWORD PTR [ecx]
	cmpeqps xmm6,xmm4
	cmpeqps xmm6,XMMWORD PTR [ecx]
	cmpltpd xmm6,xmm4
	cmpltpd xmm6,XMMWORD PTR [ecx]
	cmpltps xmm6,xmm4
	cmpltps xmm6,XMMWORD PTR [ecx]
	cmplepd xmm6,xmm4
	cmplepd xmm6,XMMWORD PTR [ecx]
	cmpleps xmm6,xmm4
	cmpleps xmm6,XMMWORD PTR [ecx]
	cmpunordpd xmm6,xmm4
	cmpunordpd xmm6,XMMWORD PTR [ecx]
	cmpunordps xmm6,xmm4
	cmpunordps xmm6,XMMWORD PTR [ecx]
	cmpneqpd xmm6,xmm4
	cmpneqpd xmm6,XMMWORD PTR [ecx]
	cmpneqps xmm6,xmm4
	cmpneqps xmm6,XMMWORD PTR [ecx]
	cmpnltpd xmm6,xmm4
	cmpnltpd xmm6,XMMWORD PTR [ecx]
	cmpnltps xmm6,xmm4
	cmpnltps xmm6,XMMWORD PTR [ecx]
	cmpnlepd xmm6,xmm4
	cmpnlepd xmm6,XMMWORD PTR [ecx]
	cmpnleps xmm6,xmm4
	cmpnleps xmm6,XMMWORD PTR [ecx]
	cmpordpd xmm6,xmm4
	cmpordpd xmm6,XMMWORD PTR [ecx]
	cmpordps xmm6,xmm4
	cmpordps xmm6,XMMWORD PTR [ecx]

# Tests for op imm8, xmm/mem128, xmm
	aeskeygenassist xmm6,xmm4,100
	aeskeygenassist xmm6,XMMWORD PTR [ecx],100
	pcmpestri xmm6,xmm4,100
	pcmpestri xmm6,XMMWORD PTR [ecx],100
	pcmpestrm xmm6,xmm4,100
	pcmpestrm xmm6,XMMWORD PTR [ecx],100
	pcmpistri xmm6,xmm4,100
	pcmpistri xmm6,XMMWORD PTR [ecx],100
	pcmpistrm xmm6,xmm4,100
	pcmpistrm xmm6,XMMWORD PTR [ecx],100
	pshufd xmm6,xmm4,100
	pshufd xmm6,XMMWORD PTR [ecx],100
	pshufhw xmm6,xmm4,100
	pshufhw xmm6,XMMWORD PTR [ecx],100
	pshuflw xmm6,xmm4,100
	pshuflw xmm6,XMMWORD PTR [ecx],100
	roundpd xmm6,xmm4,100
	roundpd xmm6,XMMWORD PTR [ecx],100
	roundps xmm6,xmm4,100
	roundps xmm6,XMMWORD PTR [ecx],100

# Tests for op imm8, xmm/mem128, xmm[, xmm]
	blendpd xmm6,xmm4,100
	blendpd xmm6,XMMWORD PTR [ecx],100
	blendps xmm6,xmm4,100
	blendps xmm6,XMMWORD PTR [ecx],100
	cmppd xmm6,xmm4,100
	cmppd xmm6,XMMWORD PTR [ecx],100
	cmpps xmm6,xmm4,100
	cmpps xmm6,XMMWORD PTR [ecx],100
	dppd xmm6,xmm4,100
	dppd xmm6,XMMWORD PTR [ecx],100
	dpps xmm6,xmm4,100
	dpps xmm6,XMMWORD PTR [ecx],100
	mpsadbw xmm6,xmm4,100
	mpsadbw xmm6,XMMWORD PTR [ecx],100
	palignr xmm6,xmm4,100
	palignr xmm6,XMMWORD PTR [ecx],100
	pblendw xmm6,xmm4,100
	pblendw xmm6,XMMWORD PTR [ecx],100
	pclmulqdq xmm6,xmm4,100
	pclmulqdq xmm6,XMMWORD PTR [ecx],100
	shufpd xmm6,xmm4,100
	shufpd xmm6,XMMWORD PTR [ecx],100
	shufps xmm6,xmm4,100
	shufps xmm6,XMMWORD PTR [ecx],100

# Tests for op xmm0, xmm/mem128, xmm[, xmm]
	blendvpd xmm6,xmm4,xmm0
	blendvpd xmm6,XMMWORD PTR [ecx],xmm0
	blendvpd xmm6,xmm4
	blendvpd xmm6,XMMWORD PTR [ecx]
	blendvps xmm6,xmm4,xmm0
	blendvps xmm6,XMMWORD PTR [ecx],xmm0
	blendvps xmm6,xmm4
	blendvps xmm6,XMMWORD PTR [ecx]
	pblendvb xmm6,xmm4,xmm0
	pblendvb xmm6,XMMWORD PTR [ecx],xmm0
	pblendvb xmm6,xmm4
	pblendvb xmm6,XMMWORD PTR [ecx]

# Tests for op xmm/mem64, xmm
	comisd xmm6,xmm4
	comisd xmm4,QWORD PTR [ecx]
	cvtdq2pd xmm6,xmm4
	cvtdq2pd xmm4,QWORD PTR [ecx]
	cvtpi2pd xmm4,QWORD PTR [ecx]
	cvtps2pd xmm6,xmm4
	cvtps2pd xmm4,QWORD PTR [ecx]
	movddup xmm6,xmm4
	movddup xmm4,QWORD PTR [ecx]
	pmovsxbw xmm6,xmm4
	pmovsxbw xmm4,QWORD PTR [ecx]
	pmovsxwd xmm6,xmm4
	pmovsxwd xmm4,QWORD PTR [ecx]
	pmovsxdq xmm6,xmm4
	pmovsxdq xmm4,QWORD PTR [ecx]
	pmovzxbw xmm6,xmm4
	pmovzxbw xmm4,QWORD PTR [ecx]
	pmovzxwd xmm6,xmm4
	pmovzxwd xmm4,QWORD PTR [ecx]
	pmovzxdq xmm6,xmm4
	pmovzxdq xmm4,QWORD PTR [ecx]
	ucomisd xmm6,xmm4
	ucomisd xmm4,QWORD PTR [ecx]

# Tests for op mem64, xmm
	movsd xmm4,QWORD PTR [ecx]

# Tests for op xmm, mem64
	movlpd QWORD PTR [ecx],xmm4
	movlps QWORD PTR [ecx],xmm4
	movhpd QWORD PTR [ecx],xmm4
	movhps QWORD PTR [ecx],xmm4
	movsd QWORD PTR [ecx],xmm4

# Tests for op xmm, regq/mem64
# Tests for op regq/mem64, xmm
	movq QWORD PTR [ecx],xmm4
	movq xmm4,QWORD PTR [ecx]

# Tests for op xmm/mem64, regl
	cvtsd2si ecx,xmm4
	cvtsd2si ecx,QWORD PTR [ecx]
	cvttsd2si ecx,xmm4
	cvttsd2si ecx,QWORD PTR [ecx]

# Tests for op mem64, xmm[, xmm]
	movlpd xmm4,QWORD PTR [ecx]
	movlps xmm4,QWORD PTR [ecx]
	movhpd xmm4,QWORD PTR [ecx]
	movhps xmm4,QWORD PTR [ecx]

# Tests for op imm8, xmm/mem64, xmm[, xmm]
	cmpsd xmm6,xmm4,100
	cmpsd xmm6,QWORD PTR [ecx],100
	roundsd xmm6,xmm4,100
	roundsd xmm6,QWORD PTR [ecx],100

# Tests for op xmm/mem64, xmm[, xmm]
	addsd xmm6,xmm4
	addsd xmm6,QWORD PTR [ecx]
	cvtsd2ss xmm6,xmm4
	cvtsd2ss xmm6,QWORD PTR [ecx]
	divsd xmm6,xmm4
	divsd xmm6,QWORD PTR [ecx]
	maxsd xmm6,xmm4
	maxsd xmm6,QWORD PTR [ecx]
	minsd xmm6,xmm4
	minsd xmm6,QWORD PTR [ecx]
	mulsd xmm6,xmm4
	mulsd xmm6,QWORD PTR [ecx]
	sqrtsd xmm6,xmm4
	sqrtsd xmm6,QWORD PTR [ecx]
	subsd xmm6,xmm4
	subsd xmm6,QWORD PTR [ecx]
	cmpeqsd xmm6,xmm4
	cmpeqsd xmm6,QWORD PTR [ecx]
	cmpltsd xmm6,xmm4
	cmpltsd xmm6,QWORD PTR [ecx]
	cmplesd xmm6,xmm4
	cmplesd xmm6,QWORD PTR [ecx]
	cmpunordsd xmm6,xmm4
	cmpunordsd xmm6,QWORD PTR [ecx]
	cmpneqsd xmm6,xmm4
	cmpneqsd xmm6,QWORD PTR [ecx]
	cmpnltsd xmm6,xmm4
	cmpnltsd xmm6,QWORD PTR [ecx]
	cmpnlesd xmm6,xmm4
	cmpnlesd xmm6,QWORD PTR [ecx]
	cmpordsd xmm6,xmm4
	cmpordsd xmm6,QWORD PTR [ecx]

# Tests for op xmm/mem32, xmm[, xmm]
	addss xmm6,xmm4
	addss xmm6,DWORD PTR [ecx]
	cvtss2sd xmm6,xmm4
	cvtss2sd xmm6,DWORD PTR [ecx]
	divss xmm6,xmm4
	divss xmm6,DWORD PTR [ecx]
	maxss xmm6,xmm4
	maxss xmm6,DWORD PTR [ecx]
	minss xmm6,xmm4
	minss xmm6,DWORD PTR [ecx]
	mulss xmm6,xmm4
	mulss xmm6,DWORD PTR [ecx]
	rcpss xmm6,xmm4
	rcpss xmm6,DWORD PTR [ecx]
	rsqrtss xmm6,xmm4
	rsqrtss xmm6,DWORD PTR [ecx]
	sqrtss xmm6,xmm4
	sqrtss xmm6,DWORD PTR [ecx]
	subss xmm6,xmm4
	subss xmm6,DWORD PTR [ecx]
	cmpeqss xmm6,xmm4
	cmpeqss xmm6,DWORD PTR [ecx]
	cmpltss xmm6,xmm4
	cmpltss xmm6,DWORD PTR [ecx]
	cmpless xmm6,xmm4
	cmpless xmm6,DWORD PTR [ecx]
	cmpunordss xmm6,xmm4
	cmpunordss xmm6,DWORD PTR [ecx]
	cmpneqss xmm6,xmm4
	cmpneqss xmm6,DWORD PTR [ecx]
	cmpnltss xmm6,xmm4
	cmpnltss xmm6,DWORD PTR [ecx]
	cmpnless xmm6,xmm4
	cmpnless xmm6,DWORD PTR [ecx]
	cmpordss xmm6,xmm4
	cmpordss xmm6,DWORD PTR [ecx]

# Tests for op xmm/mem32, xmm
	comiss xmm6,xmm4
	comiss xmm4,DWORD PTR [ecx]
	pmovsxbd xmm6,xmm4
	pmovsxbd xmm4,DWORD PTR [ecx]
	pmovsxwq xmm6,xmm4
	pmovsxwq xmm4,DWORD PTR [ecx]
	pmovzxbd xmm6,xmm4
	pmovzxbd xmm4,DWORD PTR [ecx]
	pmovzxwq xmm6,xmm4
	pmovzxwq xmm4,DWORD PTR [ecx]
	ucomiss xmm6,xmm4
	ucomiss xmm4,DWORD PTR [ecx]

# Tests for op mem32, xmm
	movss xmm4,DWORD PTR [ecx]

# Tests for op xmm, mem32
	movss DWORD PTR [ecx],xmm4

# Tests for op xmm, regl/mem32
# Tests for op regl/mem32, xmm
	movd ecx,xmm4
	movd DWORD PTR [ecx],xmm4
	movd xmm4,ecx
	movd xmm4,DWORD PTR [ecx]

# Tests for op xmm/mem32, regl
	cvtss2si ecx,xmm4
	cvtss2si ecx,DWORD PTR [ecx]
	cvttss2si ecx,xmm4
	cvttss2si ecx,DWORD PTR [ecx]

# Tests for op imm8, xmm, regq/mem32
	extractps DWORD PTR [ecx],xmm4,100
# Tests for op imm8, xmm, regl/mem32
	pextrd ecx,xmm4,100
	pextrd DWORD PTR [ecx],xmm4,100
	extractps ecx,xmm4,100
	extractps DWORD PTR [ecx],xmm4,100

# Tests for op regl/mem32, xmm[, xmm]
	cvtsi2sd xmm4,ecx
	cvtsi2sd xmm4,DWORD PTR [ecx]
	cvtsi2ss xmm4,ecx
	cvtsi2ss xmm4,DWORD PTR [ecx]

# Tests for op imm8, xmm/mem32, xmm[, xmm]
	cmpss xmm6,xmm4,100
	cmpss xmm6,DWORD PTR [ecx],100
	insertps xmm6,xmm4,100
	insertps xmm6,DWORD PTR [ecx],100
	roundss xmm6,xmm4,100
	roundss xmm6,DWORD PTR [ecx],100

# Tests for op xmm/m16, xmm
	pmovsxbq xmm6,xmm4
	pmovsxbq xmm4,WORD PTR [ecx]
	pmovzxbq xmm6,xmm4
	pmovzxbq xmm4,WORD PTR [ecx]

# Tests for op imm8, xmm, regl/mem16
	pextrw ecx,xmm4,100
	pextrw WORD PTR [ecx],xmm4,100

# Tests for op imm8, xmm, regq/mem16
	pextrw WORD PTR [ecx],xmm4,100

# Tests for op imm8, regl/mem16, xmm[, xmm]
	pinsrw xmm4,ecx,100
	pinsrw xmm4,WORD PTR [ecx],100


# Tests for op imm8, xmm, regl/mem8
	pextrb ecx,xmm4,100
	pextrb BYTE PTR [ecx],xmm4,100

# Tests for op imm8, regl/mem8, xmm[, xmm]
	pinsrb xmm4,ecx,100
	pinsrb xmm4,BYTE PTR [ecx],100

# Tests for op imm8, xmm, regq/mem8
	pextrb BYTE PTR [ecx],xmm4,100

# Tests for op imm8, regl/mem8, xmm[, xmm]
	pinsrb xmm4,ecx,100
	pinsrb xmm4,BYTE PTR [ecx],100

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

