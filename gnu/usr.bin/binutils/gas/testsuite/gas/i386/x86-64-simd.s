	.text
_start:
	addsubps 0x12345678(%rip),%xmm1
	comisd 0x12345678(%rip),%xmm1
	comiss 0x12345678(%rip),%xmm1
	cvtdq2pd 0x12345678(%rip),%xmm1
	cvtpd2dq 0x12345678(%rip),%xmm1
	cvtps2pd 0x12345678(%rip),%xmm1
	cvttps2dq 0x12345678(%rip),%xmm1
	cvtsi2ss %eax, %xmm1
	cvtsi2sd %eax, %xmm1
	cvtsi2ssl %eax, %xmm1
	cvtsi2sdl %eax, %xmm1
	cvtsi2ss %rax, %xmm1
	cvtsi2sd %rax, %xmm1
	cvtsi2ssq %rax, %xmm1
	cvtsi2sdq %rax, %xmm1
	cvtsi2ssl (%rax), %xmm1
	cvtsi2sdl (%rax), %xmm1
	cvtsi2ssq (%rax), %xmm1
	cvtsi2sdq (%rax), %xmm1
	haddps 0x12345678(%rip),%xmm1
	movdqu %xmm1,0x12345678(%rip)
	movdqu 0x12345678(%rip),%xmm1
	movhpd %xmm1,0x12345678(%rip)
	movhpd 0x12345678(%rip),%xmm1
	movhps %xmm1,0x12345678(%rip)
	movhps 0x12345678(%rip),%xmm1
	movlpd %xmm1,0x12345678(%rip)
	movlpd 0x12345678(%rip),%xmm1
	movlps %xmm1,0x12345678(%rip)
	movlps 0x12345678(%rip),%xmm1
	movq %xmm1,0x12345678(%rip)
	movq 0x12345678(%rip),%xmm1
	movshdup 0x12345678(%rip),%xmm1
	movsldup 0x12345678(%rip),%xmm1
	pshufhw $0x90,0x12345678(%rip),%xmm1
	pshuflw $0x90,0x12345678(%rip),%xmm1
	punpcklbw 0x12345678(%rip),%mm1
	punpckldq 0x12345678(%rip),%mm1
	punpcklwd 0x12345678(%rip),%mm1
	punpcklbw 0x12345678(%rip),%xmm1
	punpckldq 0x12345678(%rip),%xmm1
	punpcklwd 0x12345678(%rip),%xmm1
	punpcklqdq 0x12345678(%rip),%xmm1
	ucomisd 0x12345678(%rip),%xmm1
	ucomiss 0x12345678(%rip),%xmm1

	cmpeqsd (%rax),%xmm0
	cmpeqss (%rax),%xmm0
	cvtpi2pd (%rax),%xmm0
	cvtpi2ps (%rax),%xmm0
	cvtps2pi (%rax),%mm0
	cvtsd2si (%rax),%eax
	cvtsd2siq (%rax),%rax
	cvttsd2si (%rax),%eax
	cvttsd2siq (%rax),%rax
	cvtsd2ss (%rax),%xmm0
	cvtss2sd (%rax),%xmm0
	cvtss2si (%rax),%eax
	cvtss2siq (%rax),%rax
	cvttss2si (%rax),%eax
	cvttss2siq (%rax),%rax
	divsd (%rax),%xmm0
	divss (%rax),%xmm0
	maxsd (%rax),%xmm0
	maxss (%rax),%xmm0
	minss (%rax),%xmm0
	minss (%rax),%xmm0
	movntsd %xmm0,(%rax)
	movntss %xmm0,(%rax)
	movsd (%rax),%xmm0
	movsd %xmm0,(%rax)
	movss (%rax),%xmm0
	movss %xmm0,(%rax)
	mulsd (%rax),%xmm0
	mulss (%rax),%xmm0
	rcpss (%rax),%xmm0
	roundsd $0,(%rax),%xmm0
	roundss $0,(%rax),%xmm0
	rsqrtss (%rax),%xmm0
	sqrtsd (%rax),%xmm0
	sqrtss (%rax),%xmm0
	subsd (%rax),%xmm0
	subss (%rax),%xmm0

	pmovsxbw (%rax),%xmm0
	pmovsxbd (%rax),%xmm0
	pmovsxbq (%rax),%xmm0
	pmovsxwd (%rax),%xmm0
	pmovsxwq (%rax),%xmm0
	pmovsxdq (%rax),%xmm0
	pmovzxbw (%rax),%xmm0
	pmovzxbd (%rax),%xmm0
	pmovzxbq (%rax),%xmm0
	pmovzxwd (%rax),%xmm0
	pmovzxwq (%rax),%xmm0
	pmovzxdq (%rax),%xmm0
	insertps $0x0,(%rax),%xmm0

	unpckhpd (%rax),%xmm1
	unpckhps (%rax),%xmm1
	unpcklpd (%rax),%xmm1
	unpcklps (%rax),%xmm1

cmpss	$0x10,%xmm7,%xmm6
cmpss	$0x10,(%rax),%xmm7
cmpsd	$0x10,%xmm7,%xmm6
cmpsd	$0x10,(%rax),%xmm7

	paddq %mm1,%mm0
	paddq (%rax),%mm0
	paddq %xmm1,%xmm0
	paddq (%rax),%xmm0

	psubq %mm1,%mm0
	psubq (%rax),%mm0
	psubq %xmm1,%xmm0
	psubq (%rax),%xmm0

	pmuludq %mm1,%mm0
	pmuludq (%rax),%mm0
	pmuludq %xmm1,%xmm0
	pmuludq (%rax),%xmm0

	.intel_syntax noprefix

addsubps xmm1,XMMWORD PTR [rip+0x12345678]        
comisd xmm1,QWORD PTR [rip+0x12345678]        
comiss xmm1,DWORD PTR [rip+0x12345678]        
cvtdq2pd xmm1,QWORD PTR [rip+0x12345678]        
cvtpd2dq xmm1,XMMWORD PTR [rip+0x12345678]        
cvtps2pd xmm1,QWORD PTR [rip+0x12345678]        
cvttps2dq xmm1,XMMWORD PTR [rip+0x12345678]        
cvtsi2ss xmm1,eax
cvtsi2sd xmm1,eax
cvtsi2ssd xmm1,eax
cvtsi2sdd xmm1,eax
cvtsi2ss xmm1,rax
cvtsi2sd xmm1,rax
cvtsi2ssq xmm1,rax
cvtsi2sdq xmm1,rax
cvtsi2ss xmm1,DWORD PTR [rax]
cvtsi2sd xmm1,DWORD PTR [rax]
cvtsi2ssd xmm1,DWORD PTR [rax]
cvtsi2sdd xmm1,DWORD PTR [rax]
cvtsi2ss xmm1,QWORD PTR [rax]
cvtsi2sd xmm1,QWORD PTR [rax]
cvtsi2ssq xmm1,QWORD PTR [rax]
cvtsi2sdq xmm1,QWORD PTR [rax]
haddps xmm1,XMMWORD PTR [rip+0x12345678]        
movdqu XMMWORD PTR [rip+0x12345678],xmm1        
movdqu xmm1,XMMWORD PTR [rip+0x12345678]        
movhpd QWORD PTR [rip+0x12345678],xmm1        
movhpd xmm1,QWORD PTR [rip+0x12345678]        
movhps QWORD PTR [rip+0x12345678],xmm1        
movhps xmm1,QWORD PTR [rip+0x12345678]        
movlpd QWORD PTR [rip+0x12345678],xmm1        
movlpd xmm1,QWORD PTR [rip+0x12345678]        
movlps QWORD PTR [rip+0x12345678],xmm1        
movlps xmm1,QWORD PTR [rip+0x12345678]        
movq   QWORD PTR [rip+0x12345678],xmm1        
movq   xmm1,QWORD PTR [rip+0x12345678]        
movshdup xmm1,XMMWORD PTR [rip+0x12345678]        
movsldup xmm1,XMMWORD PTR [rip+0x12345678]        
pshufhw xmm1,XMMWORD PTR [rip+0x12345678],0x90        
pshuflw xmm1,XMMWORD PTR [rip+0x12345678],0x90        
punpcklbw mm1,DWORD PTR [rip+0x12345678]        
punpckldq mm1,DWORD PTR [rip+0x12345678]        
punpcklwd mm1,DWORD PTR [rip+0x12345678]        
punpcklbw xmm1,XMMWORD PTR [rip+0x12345678]        
punpckldq xmm1,XMMWORD PTR [rip+0x12345678]        
punpcklwd xmm1,XMMWORD PTR [rip+0x12345678]        
punpcklqdq xmm1,XMMWORD PTR [rip+0x12345678]        
ucomisd xmm1,QWORD PTR [rip+0x12345678]        
ucomiss xmm1,DWORD PTR [rip+0x12345678]        
cmpeqsd xmm0,QWORD PTR [rax]
cmpeqss xmm0,DWORD PTR [rax]
cvtpi2pd xmm0,QWORD PTR [rax]
cvtpi2ps xmm0,QWORD PTR [rax]
cvtps2pi mm0,QWORD PTR [rax]
cvtsd2si eax,QWORD PTR [rax]
cvtsd2si rax,QWORD PTR [rax]
cvttsd2si eax,QWORD PTR [rax]
cvttsd2si rax,QWORD PTR [rax]
cvtsd2ss xmm0,QWORD PTR [rax]
cvtss2sd xmm0,DWORD PTR [rax]
cvtss2si eax,DWORD PTR [rax]
cvtss2si rax,DWORD PTR [rax]
cvttss2si eax,DWORD PTR [rax]
cvttss2si rax,DWORD PTR [rax]
divsd  xmm0,QWORD PTR [rax]
divss  xmm0,DWORD PTR [rax]
maxsd  xmm0,QWORD PTR [rax]
maxss  xmm0,DWORD PTR [rax]
minss  xmm0,DWORD PTR [rax]
minss  xmm0,DWORD PTR [rax]
movntsd QWORD PTR [rax],xmm0
movntss DWORD PTR [rax],xmm0
movsd  xmm0,QWORD PTR [rax]
movsd  QWORD PTR [rax],xmm0
movss  xmm0,DWORD PTR [rax]
movss  DWORD PTR [rax],xmm0
mulsd  xmm0,QWORD PTR [rax]
mulss  xmm0,DWORD PTR [rax]
rcpss  xmm0,DWORD PTR [rax]
roundsd xmm0,QWORD PTR [rax],0x0
roundss xmm0,DWORD PTR [rax],0x0
rsqrtss xmm0,DWORD PTR [rax]
sqrtsd xmm0,QWORD PTR [rax]
sqrtss xmm0,DWORD PTR [rax]
subsd  xmm0,QWORD PTR [rax]
subss  xmm0,DWORD PTR [rax]
pmovsxbw xmm0,QWORD PTR [rax]
pmovsxbd xmm0,DWORD PTR [rax]
pmovsxbq xmm0,WORD PTR [rax]
pmovsxwd xmm0,QWORD PTR [rax]
pmovsxwq xmm0,DWORD PTR [rax]
pmovsxdq xmm0,QWORD PTR [rax]
pmovzxbw xmm0,QWORD PTR [rax]
pmovzxbd xmm0,DWORD PTR [rax]
pmovzxbq xmm0,WORD PTR [rax]
pmovzxwd xmm0,QWORD PTR [rax]
pmovzxwq xmm0,DWORD PTR [rax]
pmovzxdq xmm0,QWORD PTR [rax]
insertps xmm0,DWORD PTR [rax],0x0
unpckhpd xmm0,XMMWORD PTR [rax]
unpckhps xmm0,XMMWORD PTR [rax]
unpcklpd xmm0,XMMWORD PTR [rax]
unpcklps xmm0,XMMWORD PTR [rax]
cmpss  xmm6,xmm7,0x10
cmpss  xmm7,DWORD PTR [rax],0x10
cmpsd  xmm6,xmm7,0x10
cmpsd  xmm7,QWORD PTR [rax],0x10
paddq mm1,QWORD PTR [rax]
paddq mm1,QWORD PTR [rax]
paddq xmm1,XMMWORD PTR [rax]
paddq xmm1,XMMWORD PTR [rax]
psubq mm1,QWORD PTR [rax]
psubq mm1,QWORD PTR [rax]
psubq xmm1,XMMWORD PTR [rax]
psubq xmm1,XMMWORD PTR [rax]
pmuludq mm1,QWORD PTR [rax]
pmuludq mm1,QWORD PTR [rax]
pmuludq xmm1,XMMWORD PTR [rax]
pmuludq xmm1,XMMWORD PTR [rax]
