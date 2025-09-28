	.text
_start:
	.ifndef use16
	addsubps 0x12345678,%xmm1
	comisd 0x12345678,%xmm1
	comiss 0x12345678,%xmm1
	cvtdq2pd 0x12345678,%xmm1
	cvtpd2dq 0x12345678,%xmm1
	cvtps2pd 0x12345678,%xmm1
	cvttps2dq 0x12345678,%xmm1
	haddps 0x12345678,%xmm1
	movdqu %xmm1,0x12345678
	movdqu 0x12345678,%xmm1
	movhpd %xmm1,0x12345678
	movhpd 0x12345678,%xmm1
	movhps %xmm1,0x12345678
	movhps 0x12345678,%xmm1
	movlpd %xmm1,0x12345678
	movlpd 0x12345678,%xmm1
	movlps %xmm1,0x12345678
	movlps 0x12345678,%xmm1
	movshdup 0x12345678,%xmm1
	movsldup 0x12345678,%xmm1
	pshufhw $0x90,0x12345678,%xmm1
	pshuflw $0x90,0x12345678,%xmm1
	punpcklbw 0x12345678,%mm1
	punpckldq 0x12345678,%mm1
	punpcklwd 0x12345678,%mm1
	punpcklbw 0x12345678,%xmm1
	punpckldq 0x12345678,%xmm1
	punpcklwd 0x12345678,%xmm1
	punpcklqdq 0x12345678,%xmm1
	ucomisd 0x12345678,%xmm1
	ucomiss 0x12345678,%xmm1
	.endif

	cmpeqsd (%eax),%xmm0
	cmpeqss (%eax),%xmm0
	cvtpi2pd (%eax),%xmm0
	cvtpi2ps (%eax),%xmm0
	cvtps2pi (%eax),%mm0
	cvtsd2si (%eax),%eax
	cvttsd2si (%eax),%eax
	cvtsd2ss (%eax),%xmm0
	cvtss2sd (%eax),%xmm0
	cvtss2si (%eax),%eax
	cvttss2si (%eax),%eax
	divsd (%eax),%xmm0
	divss (%eax),%xmm0
	maxsd (%eax),%xmm0
	maxss (%eax),%xmm0
	minss (%eax),%xmm0
	minss (%eax),%xmm0
	movntsd %xmm0,(%eax)
	movntss %xmm0,(%eax)
	movsd (%eax),%xmm0
	movsd %xmm0,(%eax)
	movss (%eax),%xmm0
	movss %xmm0,(%eax)
	mulsd (%eax),%xmm0
	mulss (%eax),%xmm0
	rcpss (%eax),%xmm0
	roundsd $0,(%eax),%xmm0
	roundss $0,(%eax),%xmm0
	rsqrtss (%eax),%xmm0
	sqrtsd (%eax),%xmm0
	sqrtss (%eax),%xmm0
	subsd (%eax),%xmm0
	subss (%eax),%xmm0

	pmovsxbw (%eax),%xmm0
	pmovsxbd (%eax),%xmm0
	pmovsxbq (%eax),%xmm0
	pmovsxwd (%eax),%xmm0
	pmovsxwq (%eax),%xmm0
	pmovsxdq (%eax),%xmm0
	pmovzxbw (%eax),%xmm0
	pmovzxbd (%eax),%xmm0
	pmovzxbq (%eax),%xmm0
	pmovzxwd (%eax),%xmm0
	pmovzxwq (%eax),%xmm0
	pmovzxdq (%eax),%xmm0
	insertps $0x0,(%eax),%xmm0

	unpckhpd (%eax),%xmm1
	unpckhps (%eax),%xmm1
	unpcklpd (%eax),%xmm1
	unpcklps (%eax),%xmm1

cmpss	$0x10,%xmm7,%xmm6
cmpss	$0x10,(%eax),%xmm7
cmpsd	$0x10,%xmm7,%xmm6
cmpsd	$0x10,(%eax),%xmm7

	cvtsi2ss %eax, %xmm1
	cvtsi2sd %eax, %xmm1
	cvtsi2ssl %eax, %xmm1
	cvtsi2sdl %eax, %xmm1
	cvtsi2ss (%eax), %xmm1
	cvtsi2sd (%eax), %xmm1
	cvtsi2ssl (%eax), %xmm1
	cvtsi2sdl (%eax), %xmm1

	.intel_syntax noprefix

	.ifndef use16
addsubps xmm1,XMMWORD PTR ds:0x12345678
comisd xmm1,QWORD PTR ds:0x12345678
comiss xmm1,DWORD PTR ds:0x12345678
cvtdq2pd xmm1,QWORD PTR ds:0x12345678
cvtpd2dq xmm1,XMMWORD PTR ds:0x12345678
cvtps2pd xmm1,QWORD PTR ds:0x12345678
cvttps2dq xmm1,XMMWORD PTR ds:0x12345678
haddps xmm1,XMMWORD PTR ds:0x12345678
movdqu XMMWORD PTR ds:0x12345678,xmm1
movdqu xmm1,XMMWORD PTR ds:0x12345678
movhpd QWORD PTR ds:0x12345678,xmm1
movhpd xmm1,QWORD PTR ds:0x12345678
movhps QWORD PTR ds:0x12345678,xmm1
movhps xmm1,QWORD PTR ds:0x12345678
movlpd QWORD PTR ds:0x12345678,xmm1
movlpd xmm1,QWORD PTR ds:0x12345678
movlps QWORD PTR ds:0x12345678,xmm1
movlps xmm1,QWORD PTR ds:0x12345678
movshdup xmm1,XMMWORD PTR ds:0x12345678
movsldup xmm1,XMMWORD PTR ds:0x12345678
pshufhw xmm1,XMMWORD PTR ds:0x12345678,0x90
pshuflw xmm1,XMMWORD PTR ds:0x12345678,0x90
punpcklbw mm1,DWORD PTR ds:0x12345678
punpckldq mm1,DWORD PTR ds:0x12345678
punpcklwd mm1,DWORD PTR ds:0x12345678
punpcklbw xmm1,XMMWORD PTR ds:0x12345678
punpckldq xmm1,XMMWORD PTR ds:0x12345678
punpcklwd xmm1,XMMWORD PTR ds:0x12345678
punpcklqdq xmm1,XMMWORD PTR ds:0x12345678
ucomisd xmm1,QWORD PTR ds:0x12345678
ucomiss xmm1,DWORD PTR ds:0x12345678
	.endif

cmpeqsd xmm0,QWORD PTR [eax]
cmpeqss xmm0,DWORD PTR [eax]
cvtpi2pd xmm0,QWORD PTR [eax]
cvtpi2ps xmm0,QWORD PTR [eax]
cvtps2pi mm0,QWORD PTR [eax]
cvtsd2si eax,QWORD PTR [eax]
cvttsd2si eax,QWORD PTR [eax]
cvtsd2ss xmm0,QWORD PTR [eax]
cvtss2sd xmm0,DWORD PTR [eax]
cvtss2si eax,DWORD PTR [eax]
cvttss2si eax,DWORD PTR [eax]
divsd  xmm0,QWORD PTR [eax]
divss  xmm0,DWORD PTR [eax]
maxsd  xmm0,QWORD PTR [eax]
maxss  xmm0,DWORD PTR [eax]
minss  xmm0,DWORD PTR [eax]
minss  xmm0,DWORD PTR [eax]
movntsd QWORD PTR [eax],xmm0
movntss DWORD PTR [eax],xmm0
movsd  xmm0,QWORD PTR [eax]
movsd  QWORD PTR [eax],xmm0
movss  xmm0,DWORD PTR [eax]
movss  DWORD PTR [eax],xmm0
mulsd  xmm0,QWORD PTR [eax]
mulss  xmm0,DWORD PTR [eax]
rcpss  xmm0,DWORD PTR [eax]
roundsd xmm0,QWORD PTR [eax],0x0
roundss xmm0,DWORD PTR [eax],0x0
rsqrtss xmm0,DWORD PTR [eax]
sqrtsd xmm0,QWORD PTR [eax]
sqrtss xmm0,DWORD PTR [eax]
subsd  xmm0,QWORD PTR [eax]
subss  xmm0,DWORD PTR [eax]
pmovsxbw xmm0,QWORD PTR [eax]
pmovsxbd xmm0,DWORD PTR [eax]
pmovsxbq xmm0,WORD PTR [eax]
pmovsxwd xmm0,QWORD PTR [eax]
pmovsxwq xmm0,DWORD PTR [eax]
pmovsxdq xmm0,QWORD PTR [eax]
pmovzxbw xmm0,QWORD PTR [eax]
pmovzxbd xmm0,DWORD PTR [eax]
pmovzxbq xmm0,WORD PTR [eax]
pmovzxwd xmm0,QWORD PTR [eax]
pmovzxwq xmm0,DWORD PTR [eax]
pmovzxdq xmm0,QWORD PTR [eax]
insertps xmm0,DWORD PTR [eax],0x0
unpckhpd xmm0,XMMWORD PTR [eax]
unpckhps xmm0,XMMWORD PTR [eax]
unpcklpd xmm0,XMMWORD PTR [eax]
unpcklps xmm0,XMMWORD PTR [eax]
cmpss  xmm6,xmm7,0x10
cmpss  xmm7,DWORD PTR [eax],0x10
cmpsd  xmm6,xmm7,0x10
cmpsd  xmm7,QWORD PTR [eax],0x10
cvtsi2ss xmm1,eax
cvtsi2sd xmm1,eax
cvtsi2ssd xmm1,eax
cvtsi2sdd xmm1,eax
cvtsi2ss xmm1,DWORD PTR [eax]
cvtsi2ss xmm1,[eax]
cvtsi2sd xmm1,DWORD PTR [eax]
cvtsi2sd xmm1,[eax]
cvtsi2ssd xmm1,DWORD PTR [eax]
cvtsi2sdd xmm1,DWORD PTR [eax]
cvttps2pi mm0,QWORD PTR[eax]
