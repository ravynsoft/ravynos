	.text
	.intel_syntax noprefix
xmmword:
	addsd		xmm0, xmmword ptr [eax]
	vaddsd		xmm0, xmm0, xmmword ptr [eax]
	vaddsd		xmm0{k7}, xmm0, xmmword ptr [eax]

	addss		xmm0, xmmword ptr [eax]
	vaddss		xmm0, xmm0, xmmword ptr [eax]
	vaddss		xmm0{k7}, xmm0, xmmword ptr [eax]

	vbroadcastf32x2	ymm0, xmmword ptr [eax]
	vbroadcastf32x2	zmm0, xmmword ptr [eax]

	vbroadcasti32x2	xmm0, xmmword ptr [eax]
	vbroadcasti32x2	ymm0, xmmword ptr [eax]
	vbroadcasti32x2	zmm0, xmmword ptr [eax]

	vbroadcastsd	ymm0, xmmword ptr [eax]
	vbroadcastsd	ymm0{k7}, xmmword ptr [eax]
	vbroadcastsd	zmm0{k7}, xmmword ptr [eax]

	vbroadcastss	xmm0, xmmword ptr [eax]
	vbroadcastss	xmm0{k7}, xmmword ptr [eax]
	vbroadcastss	ymm0, xmmword ptr [eax]
	vbroadcastss	ymm0{k7}, xmmword ptr [eax]
	vbroadcastss	zmm0, xmmword ptr [eax]

	cvtdq2pd	xmm0, xmmword ptr [eax]
	vcvtdq2pd	xmm0, xmmword ptr [eax]
	vcvtdq2pd	xmm0{k7}, xmmword ptr [eax]

	vcvtph2ps	xmm0, xmmword ptr [eax]
	vcvtph2ps	xmm0{k7}, xmmword ptr [eax]

	cvtps2pd	xmm0, xmmword ptr [eax]
	vcvtps2pd	xmm0, xmmword ptr [eax]
	vcvtps2pd	xmm0{k7}, xmmword ptr [eax]

	vcvtps2ph	xmmword ptr [eax], xmm0, 0
	vcvtps2ph	xmmword ptr [eax]{k7}, xmm0, 0

	vcvtudq2pd	xmm0, xmmword ptr [eax]

	insertps	xmm0, xmmword ptr [eax], 0
	vinsertps	xmm0, xmm0, xmmword ptr [eax], 0
	{evex} vinsertps xmm0, xmm0, xmmword ptr [eax], 0

	movddup		xmm0, xmmword ptr [eax]
	vmovddup	xmm0, xmmword ptr [eax]
	vmovddup	xmm0{k7}, xmmword ptr [eax]

	vpbroadcastb	xmm0, xmmword ptr [eax]
	vpbroadcastb	xmm0{k7}, xmmword ptr [eax]
	vpbroadcastb	ymm0, xmmword ptr [eax]
	vpbroadcastb	ymm0{k7}, xmmword ptr [eax]
	vpbroadcastb	zmm0, xmmword ptr [eax]

	vpbroadcastd	xmm0, xmmword ptr [eax]
	vpbroadcastd	xmm0{k7}, xmmword ptr [eax]
	vpbroadcastd	ymm0, xmmword ptr [eax]
	vpbroadcastd	ymm0{k7}, xmmword ptr [eax]
	vpbroadcastd	zmm0, xmmword ptr [eax]

	vpbroadcastq	xmm0, xmmword ptr [eax]
	vpbroadcastq	xmm0{k7}, xmmword ptr [eax]
	vpbroadcastq	ymm0, xmmword ptr [eax]
	vpbroadcastq	ymm0{k7}, xmmword ptr [eax]
	vpbroadcastq	zmm0, xmmword ptr [eax]

	vpbroadcastw	xmm0, xmmword ptr [eax]
	vpbroadcastw	xmm0{k7}, xmmword ptr [eax]
	vpbroadcastw	ymm0, xmmword ptr [eax]
	vpbroadcastw	ymm0{k7}, xmmword ptr [eax]
	vpbroadcastw	zmm0, xmmword ptr [eax]

	pmovsxbd	xmm0, xmmword ptr [eax]
	vpmovsxbd	xmm0, xmmword ptr [eax]
	vpmovsxbd	xmm0{k7}, xmmword ptr [eax]
	vpmovsxbd	ymm0, xmmword ptr [eax]
	vpmovsxbd	ymm0{k7}, xmmword ptr [eax]

	pmovsxbq	xmm0, xmmword ptr [eax]
	vpmovsxbq	xmm0, xmmword ptr [eax]
	vpmovsxbq	xmm0{k7}, xmmword ptr [eax]
	vpmovsxbq	ymm0, xmmword ptr [eax]
	vpmovsxbq	ymm0{k7}, xmmword ptr [eax]
	vpmovsxbq	zmm0, xmmword ptr [eax]

	pmovsxdq	xmm0, xmmword ptr [eax]
	vpmovsxdq	xmm0, xmmword ptr [eax]
	vpmovsxdq	xmm0{k7}, xmmword ptr [eax]

	pmovsxwd	xmm0, xmmword ptr [eax]
	vpmovsxwd	xmm0, xmmword ptr [eax]
	vpmovsxwd	xmm0{k7}, xmmword ptr [eax]

	pmovsxwq	xmm0, xmmword ptr [eax]
	vpmovsxwq	xmm0, xmmword ptr [eax]
	vpmovsxwq	xmm0{k7}, xmmword ptr [eax]
	vpmovsxwq	ymm0, xmmword ptr [eax]
	vpmovsxwq	ymm0{k7}, xmmword ptr [eax]

	pmovzxbd	xmm0, xmmword ptr [eax]
	vpmovzxbd	xmm0, xmmword ptr [eax]
	vpmovzxbd	xmm0{k7}, xmmword ptr [eax]
	vpmovzxbd	ymm0, xmmword ptr [eax]
	vpmovzxbd	ymm0{k7}, xmmword ptr [eax]

	pmovzxbq	xmm0, xmmword ptr [eax]
	vpmovzxbq	xmm0, xmmword ptr [eax]
	vpmovzxbq	xmm0{k7}, xmmword ptr [eax]
	vpmovzxbq	ymm0, xmmword ptr [eax]
	vpmovzxbq	ymm0{k7}, xmmword ptr [eax]
	vpmovzxbq	zmm0, xmmword ptr [eax]

	pmovzxdq	xmm0, xmmword ptr [eax]
	vpmovzxdq	xmm0, xmmword ptr [eax]
	vpmovzxdq	xmm0{k7}, xmmword ptr [eax]

	pmovzxwd	xmm0, xmmword ptr [eax]
	vpmovzxwd	xmm0, xmmword ptr [eax]
	vpmovzxwd	xmm0{k7}, xmmword ptr [eax]

	pmovzxwq	xmm0, xmmword ptr [eax]
	vpmovzxwq	xmm0, xmmword ptr [eax]
	vpmovzxwq	xmm0{k7}, xmmword ptr [eax]
	vpmovzxwq	ymm0, xmmword ptr [eax]
	vpmovzxwq	ymm0{k7}, xmmword ptr [eax]

	vcvtps2qq	xmm0, xmmword ptr [rax]
	vcvtps2uqq	xmm0, xmmword ptr [rax]
	vcvttps2qq	xmm0, xmmword ptr [rax]
	vcvttps2uqq	xmm0, xmmword ptr [rax]

	movq		xmm0, xmmword ptr [eax]
	vmovq		xmm0, xmmword ptr [eax]
	{evex} vmovq	xmm0, xmmword ptr [eax]

	movq		xmmword ptr [eax], xmm0
	vmovq		xmmword ptr [eax], xmm0
	{evex} vmovq	xmmword ptr [eax], xmm0

	cvtps2pi	mm0, xmmword ptr [eax]

	cvttps2pi	mm0, xmmword ptr [eax]

	vcvtph2dq	xmm0, xmmword ptr [eax]
	vcvtph2pd	xmm0, xmmword ptr [eax]
	vcvtph2psx	xmm0, xmmword ptr [eax]
	vcvtph2qq	xmm0, xmmword ptr [eax]
	vcvtph2udq	xmm0, xmmword ptr [eax]
	vcvtph2uqq	xmm0, xmmword ptr [eax]
	vcvttph2dq	xmm0, xmmword ptr [eax]
	vcvttph2qq	xmm0, xmmword ptr [eax]
	vcvttph2udq	xmm0, xmmword ptr [eax]
	vcvttph2uqq	xmm0, xmmword ptr [eax]
