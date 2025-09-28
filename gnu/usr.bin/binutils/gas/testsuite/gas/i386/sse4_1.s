# Streaming SIMD extensions 4.1 Instructions

	.text
foo:
	blendpd		$0,(%ecx),%xmm0
	blendpd		$0,%xmm1,%xmm0
	blendps		$0,(%ecx),%xmm0
	blendps		$0,%xmm1,%xmm0
	blendvpd	%xmm0,(%ecx),%xmm0
	blendvpd	%xmm0,%xmm1,%xmm0
	blendvpd	(%ecx),%xmm0
	blendvpd	%xmm1,%xmm0
	blendvps	%xmm0,(%ecx),%xmm0
	blendvps	%xmm0,%xmm1,%xmm0
	blendvps	(%ecx),%xmm0
	blendvps	%xmm1,%xmm0
	dppd		$0,(%ecx),%xmm0
	dppd		$0,%xmm1,%xmm0
	dpps		$0,(%ecx),%xmm0
	dpps		$0,%xmm1,%xmm0
	extractps	$0,%xmm0,%ecx
	extractps	$0,%xmm0,(%ecx)
	insertps	$0,%xmm1,%xmm0
	insertps	$0,(%ecx),%xmm0
	movntdqa	(%ecx),%xmm0
	mpsadbw		$0,(%ecx),%xmm0
	mpsadbw		$0,%xmm1,%xmm0
	packusdw	(%ecx),%xmm0
	packusdw	%xmm1,%xmm0
	pblendvb	%xmm0,(%ecx),%xmm0
	pblendvb	%xmm0,%xmm1,%xmm0
	pblendvb	(%ecx),%xmm0
	pblendvb	%xmm1,%xmm0
	pblendw		$0,(%ecx),%xmm0
	pblendw		$0,%xmm1,%xmm0
	pcmpeqq		%xmm1,%xmm0
	pcmpeqq		(%ecx),%xmm0
	pextrb          $0,%xmm0,%ecx
	pextrb          $0,%xmm0,(%ecx)
	pextrd          $0,%xmm0,%ecx
	pextrd          $0,%xmm0,(%ecx)
	pextrw          $0,%xmm0,%ecx
	pextrw          $0,%xmm0,(%ecx)
	phminposuw	%xmm1,%xmm0
	phminposuw	(%ecx),%xmm0
	pinsrb		$0,(%ecx),%xmm0
	pinsrb		$0,%ecx,%xmm0
	pinsrd		$0,(%ecx),%xmm0
	pinsrd		$0,%ecx,%xmm0
	pmaxsb		%xmm1,%xmm0
	pmaxsb		(%ecx),%xmm0
	pmaxsd		%xmm1,%xmm0
	pmaxsd		(%ecx),%xmm0
	pmaxud		%xmm1,%xmm0
	pmaxud		(%ecx),%xmm0
	pmaxuw		%xmm1,%xmm0
	pmaxuw		(%ecx),%xmm0
	pminsb		%xmm1,%xmm0
	pminsb		(%ecx),%xmm0
	pminsd		%xmm1,%xmm0
	pminsd		(%ecx),%xmm0
	pminud		%xmm1,%xmm0
	pminud		(%ecx),%xmm0
	pminuw		%xmm1,%xmm0
	pminuw		(%ecx),%xmm0
	pmovsxbw	%xmm1,%xmm0
	pmovsxbw	(%ecx),%xmm0
	pmovsxbd	%xmm1,%xmm0
	pmovsxbd	(%ecx),%xmm0
	pmovsxbq	%xmm1,%xmm0
	pmovsxbq	(%ecx),%xmm0
	pmovsxwd	%xmm1,%xmm0
	pmovsxwd	(%ecx),%xmm0
	pmovsxwq	%xmm1,%xmm0
	pmovsxwq	(%ecx),%xmm0
	pmovsxdq	%xmm1,%xmm0
	pmovsxdq	(%ecx),%xmm0
	pmovzxbw	%xmm1,%xmm0
	pmovzxbw	(%ecx),%xmm0
	pmovzxbd	%xmm1,%xmm0
	pmovzxbd	(%ecx),%xmm0
	pmovzxbq	%xmm1,%xmm0
	pmovzxbq	(%ecx),%xmm0
	pmovzxwd	%xmm1,%xmm0
	pmovzxwd	(%ecx),%xmm0
	pmovzxwq	%xmm1,%xmm0
	pmovzxwq	(%ecx),%xmm0
	pmovzxdq	%xmm1,%xmm0
	pmovzxdq	(%ecx),%xmm0
	pmuldq		%xmm1,%xmm0
	pmuldq		(%ecx),%xmm0
	pmulld		%xmm1,%xmm0
	pmulld		(%ecx),%xmm0
	ptest		%xmm1,%xmm0
	ptest		(%ecx),%xmm0
	roundpd		$0,(%ecx),%xmm0
	roundpd		$0,%xmm1,%xmm0
	roundps		$0,(%ecx),%xmm0
	roundps		$0,%xmm1,%xmm0
	roundsd		$0,(%ecx),%xmm0
	roundsd		$0,%xmm1,%xmm0
	roundss		$0,(%ecx),%xmm0
	roundss		$0,%xmm1,%xmm0

	.intel_syntax noprefix
	blendpd xmm0,XMMWORD PTR [ecx],0x0
	blendpd xmm0,xmm1,0x0
	blendps xmm0,XMMWORD PTR [ecx],0x0
	blendps xmm0,xmm1,0x0
	blendvpd xmm0,XMMWORD PTR [ecx],xmm0
	blendvpd xmm0,xmm1,xmm0
	blendvps xmm0,XMMWORD PTR [ecx],xmm0
	blendvps xmm0,xmm1,xmm0
	dppd   xmm0,XMMWORD PTR [ecx],0x0
	dppd   xmm0,xmm1,0x0
	dpps   xmm0,XMMWORD PTR [ecx],0x0
	dpps   xmm0,xmm1,0x0
	extractps ecx,xmm0,0x0
	extractps DWORD PTR [ecx],xmm0,0x0
	insertps xmm0,xmm1,0x0
	insertps xmm0,DWORD PTR [ecx],0x0
	movntdqa xmm0,XMMWORD PTR [ecx]
	mpsadbw xmm0,XMMWORD PTR [ecx],0x0
	mpsadbw xmm0,xmm1,0x0
	packusdw xmm0,XMMWORD PTR [ecx]
	packusdw xmm0,xmm1
	pblendvb xmm0,XMMWORD PTR [ecx],xmm0
	pblendvb xmm0,xmm1,xmm0
	pblendw xmm0,XMMWORD PTR [ecx],0x0
	pblendw xmm0,xmm1,0x0
	pcmpeqq xmm0,xmm1
	pcmpeqq xmm0,XMMWORD PTR [ecx]
	pextrb ecx,xmm0,0x0
	pextrb BYTE PTR [ecx],xmm0,0x0
	pextrd ecx,xmm0,0x0
	pextrd DWORD PTR [ecx],xmm0,0x0
	pextrw ecx,xmm0,0x0
	pextrw WORD PTR [ecx],xmm0,0x0
	phminposuw xmm0,xmm1
	phminposuw xmm0,XMMWORD PTR [ecx]
	pinsrb xmm0,BYTE PTR [ecx],0x0
	pinsrb xmm0,ecx,0x0
	pinsrd xmm0,DWORD PTR [ecx],0x0
	pinsrd xmm0,ecx,0x0
	pmaxsb xmm0,xmm1
	pmaxsb xmm0,XMMWORD PTR [ecx]
	pmaxsd xmm0,xmm1
	pmaxsd xmm0,XMMWORD PTR [ecx]
	pmaxud xmm0,xmm1
	pmaxud xmm0,XMMWORD PTR [ecx]
	pmaxuw xmm0,xmm1
	pmaxuw xmm0,XMMWORD PTR [ecx]
	pminsb xmm0,xmm1
	pminsb xmm0,XMMWORD PTR [ecx]
	pminsd xmm0,xmm1
	pminsd xmm0,XMMWORD PTR [ecx]
	pminud xmm0,xmm1
	pminud xmm0,XMMWORD PTR [ecx]
	pminuw xmm0,xmm1
	pminuw xmm0,XMMWORD PTR [ecx]
	pmovsxbw xmm0,xmm1
	pmovsxbw xmm0,QWORD PTR [ecx]
	pmovsxbd xmm0,xmm1
	pmovsxbd xmm0,DWORD PTR [ecx]
	pmovsxbq xmm0,xmm1
	pmovsxbq xmm0,WORD PTR [ecx]
	pmovsxwd xmm0,xmm1
	pmovsxwd xmm0,QWORD PTR [ecx]
	pmovsxwq xmm0,xmm1
	pmovsxwq xmm0,DWORD PTR [ecx]
	pmovsxdq xmm0,xmm1
	pmovsxdq xmm0,QWORD PTR [ecx]
	pmovzxbw xmm0,xmm1
	pmovzxbw xmm0,QWORD PTR [ecx]
	pmovzxbd xmm0,xmm1
	pmovzxbd xmm0,DWORD PTR [ecx]
	pmovzxbq xmm0,xmm1
	pmovzxbq xmm0,WORD PTR [ecx]
	pmovzxwd xmm0,xmm1
	pmovzxwd xmm0,QWORD PTR [ecx]
	pmovzxwq xmm0,xmm1
	pmovzxwq xmm0,DWORD PTR [ecx]
	pmovzxdq xmm0,xmm1
	pmovzxdq xmm0,QWORD PTR [ecx]
	pmuldq xmm0,xmm1
	pmuldq xmm0,XMMWORD PTR [ecx]
	pmulld xmm0,xmm1
	pmulld xmm0,XMMWORD PTR [ecx]
	ptest  xmm0,xmm1
	ptest  xmm0,XMMWORD PTR [ecx]
	roundpd xmm0,XMMWORD PTR [ecx],0x0
	roundpd xmm0,xmm1,0x0
	roundps xmm0,XMMWORD PTR [ecx],0x0
	roundps xmm0,xmm1,0x0
	roundsd xmm0,QWORD PTR [ecx],0x0
	roundsd xmm0,xmm1,0x0
	roundss xmm0,DWORD PTR [ecx],0x0
	roundss xmm0,xmm1,0x0

	.p2align	4,0
