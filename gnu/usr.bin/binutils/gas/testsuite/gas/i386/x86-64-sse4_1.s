# Streaming SIMD extensions 4.1 Instructions

	.text
foo:
	blendpd		$0x0,(%rcx),%xmm0
	blendpd		$0x0,%xmm1,%xmm0
	blendps		$0x0,(%rcx),%xmm0
	blendps		$0x0,%xmm1,%xmm0
	blendvpd	%xmm0,(%rcx),%xmm0
	blendvpd	%xmm0,%xmm1,%xmm0
	blendvpd	(%rcx),%xmm0
	blendvpd	%xmm1,%xmm0
	blendvps	%xmm0,(%rcx),%xmm0
	blendvps	%xmm0,%xmm1,%xmm0
	blendvps	(%rcx),%xmm0
	blendvps	%xmm1,%xmm0
	dppd		$0x0,(%rcx),%xmm0
	dppd		$0x0,%xmm1,%xmm0
	dpps		$0x0,(%rcx),%xmm0
	dpps		$0x0,%xmm1,%xmm0
	extractps	$0x0,%xmm0,%rcx
	extractps	$0x0,%xmm0,%ecx
	extractps	$0x0,%xmm0,(%rcx)
	insertps	$0x0,%xmm1,%xmm0
	insertps	$0x0,(%rcx),%xmm0
	movntdqa	(%rcx),%xmm0
	mpsadbw		$0x0,(%rcx),%xmm0
	mpsadbw		$0x0,%xmm1,%xmm0
	packusdw	(%rcx),%xmm0
	packusdw	%xmm1,%xmm0
	pblendvb	%xmm0,(%rcx),%xmm0
	pblendvb	%xmm0,%xmm1,%xmm0
	pblendvb	(%rcx),%xmm0
	pblendvb	%xmm1,%xmm0
	pblendw		$0x0,(%rcx),%xmm0
	pblendw		$0x0,%xmm1,%xmm0
	pcmpeqq		%xmm1,%xmm0
	pcmpeqq		(%rcx),%xmm0
	pextrb          $0x0,%xmm0,%rcx
	pextrb          $0x0,%xmm0,%ecx
	pextrb          $0x0,%xmm0,(%rcx)
	pextrd          $0x0,%xmm0,%ecx
	pextrd          $0x0,%xmm0,(%rcx)
	pextrq          $0x0,%xmm0,%rcx
	pextrq          $0x0,%xmm0,(%rcx)
	pextrw          $0x0,%xmm0,%rcx
	pextrw          $0x0,%xmm0,%ecx
	pextrw          $0x0,%xmm0,(%rcx)
	phminposuw	%xmm1,%xmm0
	phminposuw	(%rcx),%xmm0
	pinsrb		$0x0,(%rcx),%xmm0
	pinsrb		$0x0,%ecx,%xmm0
	pinsrb		$0x0,%rcx,%xmm0
	pinsrd		$0x0,(%rcx),%xmm0
	pinsrd		$0x0,%ecx,%xmm0
	pinsrq		$0x0,(%rcx),%xmm0
	pinsrq		$0x0,%rcx,%xmm0
	pmaxsb		%xmm1,%xmm0
	pmaxsb		(%rcx),%xmm0
	pmaxsd		%xmm1,%xmm0
	pmaxsd		(%rcx),%xmm0
	pmaxud		%xmm1,%xmm0
	pmaxud		(%rcx),%xmm0
	pmaxuw		%xmm1,%xmm0
	pmaxuw		(%rcx),%xmm0
	pminsb		%xmm1,%xmm0
	pminsb		(%rcx),%xmm0
	pminsd		%xmm1,%xmm0
	pminsd		(%rcx),%xmm0
	pminud		%xmm1,%xmm0
	pminud		(%rcx),%xmm0
	pminuw		%xmm1,%xmm0
	pminuw		(%rcx),%xmm0
	pmovsxbw	%xmm1,%xmm0
	pmovsxbw	(%rcx),%xmm0
	pmovsxbd	%xmm1,%xmm0
	pmovsxbd	(%rcx),%xmm0
	pmovsxbq	%xmm1,%xmm0
	pmovsxbq	(%rcx),%xmm0
	pmovsxwd	%xmm1,%xmm0
	pmovsxwd	(%rcx),%xmm0
	pmovsxwq	%xmm1,%xmm0
	pmovsxwq	(%rcx),%xmm0
	pmovsxdq	%xmm1,%xmm0
	pmovsxdq	(%rcx),%xmm0
	pmovzxbw	%xmm1,%xmm0
	pmovzxbw	(%rcx),%xmm0
	pmovzxbd	%xmm1,%xmm0
	pmovzxbd	(%rcx),%xmm0
	pmovzxbq	%xmm1,%xmm0
	pmovzxbq	(%rcx),%xmm0
	pmovzxwd	%xmm1,%xmm0
	pmovzxwd	(%rcx),%xmm0
	pmovzxwq	%xmm1,%xmm0
	pmovzxwq	(%rcx),%xmm0
	pmovzxdq	%xmm1,%xmm0
	pmovzxdq	(%rcx),%xmm0
	pmuldq		%xmm1,%xmm0
	pmuldq		(%rcx),%xmm0
	pmulld		%xmm1,%xmm0
	pmulld		(%rcx),%xmm0
	ptest		%xmm1,%xmm0
	ptest		(%rcx),%xmm0
	roundpd		$0x0,(%rcx),%xmm0
	roundpd		$0x0,%xmm1,%xmm0
	roundps		$0x0,(%rcx),%xmm0
	roundps		$0x0,%xmm1,%xmm0
	roundsd		$0x0,(%rcx),%xmm0
	roundsd		$0x0,%xmm1,%xmm0
	roundss		$0x0,(%rcx),%xmm0
	roundss		$0x0,%xmm1,%xmm0

	.intel_syntax noprefix
	blendpd xmm0,XMMWORD PTR [rcx],0x0
	blendpd xmm0,xmm1,0x0
	blendps xmm0,XMMWORD PTR [rcx],0x0
	blendps xmm0,xmm1,0x0
	blendvpd xmm0,XMMWORD PTR [rcx],xmm0
	blendvpd xmm0,xmm1,xmm0
	blendvps xmm0,XMMWORD PTR [rcx],xmm0
	blendvps xmm0,xmm1,xmm0
	dppd   xmm0,XMMWORD PTR [rcx],0x0
	dppd   xmm0,xmm1,0x0
	dpps   xmm0,XMMWORD PTR [rcx],0x0
	dpps   xmm0,xmm1,0x0
	extractps rcx,xmm0,0x0
	extractps ecx,xmm0,0x0
	extractps DWORD PTR [rcx],xmm0,0x0
	insertps xmm0,xmm1,0x0
	insertps xmm0,DWORD PTR [rcx],0x0
	movntdqa xmm0,XMMWORD PTR [rcx]
	mpsadbw xmm0,XMMWORD PTR [rcx],0x0
	mpsadbw xmm0,xmm1,0x0
	packusdw xmm0,XMMWORD PTR [rcx]
	packusdw xmm0,xmm1
	pblendvb xmm0,XMMWORD PTR [rcx],xmm0
	pblendvb xmm0,xmm1,xmm0
	pblendw xmm0,XMMWORD PTR [rcx],0x0
	pblendw xmm0,xmm1,0x0
	pcmpeqq xmm0,xmm1
	pcmpeqq xmm0,XMMWORD PTR [rcx]
	pextrb rcx,xmm0,0x0
	pextrb ecx,xmm0,0x0
	pextrb BYTE PTR [rcx],xmm0,0x0
	pextrd ecx,xmm0,0x0
	pextrd DWORD PTR [rcx],xmm0,0x0
	pextrq rcx,xmm0,0x0
	pextrq QWORD PTR [rcx],xmm0,0x0
	pextrw rcx,xmm0,0x0
	pextrw ecx,xmm0,0x0
	pextrw WORD PTR [rcx],xmm0,0x0
	phminposuw xmm0,xmm1
	phminposuw xmm0,XMMWORD PTR [rcx]
	pinsrb xmm0,BYTE PTR [rcx],0x0
	pinsrb xmm0,ecx,0x0
	pinsrb xmm0,rcx,0x0
	pinsrd xmm0,DWORD PTR [rcx],0x0
	pinsrd xmm0,ecx,0x0
	pinsrq xmm0,QWORD PTR [rcx],0x0
	pinsrq xmm0,rcx,0x0
	pmaxsb xmm0,xmm1
	pmaxsb xmm0,XMMWORD PTR [rcx]
	pmaxsd xmm0,xmm1
	pmaxsd xmm0,XMMWORD PTR [rcx]
	pmaxud xmm0,xmm1
	pmaxud xmm0,XMMWORD PTR [rcx]
	pmaxuw xmm0,xmm1
	pmaxuw xmm0,XMMWORD PTR [rcx]
	pminsb xmm0,xmm1
	pminsb xmm0,XMMWORD PTR [rcx]
	pminsd xmm0,xmm1
	pminsd xmm0,XMMWORD PTR [rcx]
	pminud xmm0,xmm1
	pminud xmm0,XMMWORD PTR [rcx]
	pminuw xmm0,xmm1
	pminuw xmm0,XMMWORD PTR [rcx]
	pmovsxbw xmm0,xmm1
	pmovsxbw xmm0,QWORD PTR [rcx]
	pmovsxbd xmm0,xmm1
	pmovsxbd xmm0,DWORD PTR [rcx]
	pmovsxbq xmm0,xmm1
	pmovsxbq xmm0,WORD PTR [rcx]
	pmovsxwd xmm0,xmm1
	pmovsxwd xmm0,QWORD PTR [rcx]
	pmovsxwq xmm0,xmm1
	pmovsxwq xmm0,DWORD PTR [rcx]
	pmovsxdq xmm0,xmm1
	pmovsxdq xmm0,QWORD PTR [rcx]
	pmovzxbw xmm0,xmm1
	pmovzxbw xmm0,QWORD PTR [rcx]
	pmovzxbd xmm0,xmm1
	pmovzxbd xmm0,DWORD PTR [rcx]
	pmovzxbq xmm0,xmm1
	pmovzxbq xmm0,WORD PTR [rcx]
	pmovzxwd xmm0,xmm1
	pmovzxwd xmm0,QWORD PTR [rcx]
	pmovzxwq xmm0,xmm1
	pmovzxwq xmm0,DWORD PTR [rcx]
	pmovzxdq xmm0,xmm1
	pmovzxdq xmm0,QWORD PTR [rcx]
	pmuldq xmm0,xmm1
	pmuldq xmm0,XMMWORD PTR [rcx]
	pmulld xmm0,xmm1
	pmulld xmm0,XMMWORD PTR [rcx]
	ptest  xmm0,xmm1
	ptest  xmm0,XMMWORD PTR [rcx]
	roundpd xmm0,XMMWORD PTR [rcx],0x0
	roundpd xmm0,xmm1,0x0
	roundps xmm0,XMMWORD PTR [rcx],0x0
	roundps xmm0,xmm1,0x0
	roundsd xmm0,QWORD PTR [rcx],0x0
	roundsd xmm0,xmm1,0x0
	roundss xmm0,DWORD PTR [rcx],0x0
	roundss xmm0,xmm1,0x0

	.p2align	4,0
