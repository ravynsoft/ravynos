#as: -J
#objdump: -dw
#name: i386 SIMD

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	f2 0f d0 0d 78 56 34 12 	addsubps 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	66 0f 2f 0d 78 56 34 12 	comisd 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	0f 2f 0d 78 56 34 12 	comiss 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	f3 0f e6 0d 78 56 34 12 	cvtdq2pd 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	f2 0f e6 0d 78 56 34 12 	cvtpd2dq 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	0f 5a 0d 78 56 34 12 	cvtps2pd 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	f3 0f 5b 0d 78 56 34 12 	cvttps2dq 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	f2 0f 7c 0d 78 56 34 12 	haddps 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	f3 0f 7f 0d 78 56 34 12 	movdqu %xmm1,0x12345678
[ 	]*[a-f0-9]+:	f3 0f 6f 0d 78 56 34 12 	movdqu 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	66 0f 17 0d 78 56 34 12 	movhpd %xmm1,0x12345678
[ 	]*[a-f0-9]+:	66 0f 16 0d 78 56 34 12 	movhpd 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	0f 17 0d 78 56 34 12 	movhps %xmm1,0x12345678
[ 	]*[a-f0-9]+:	0f 16 0d 78 56 34 12 	movhps 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	66 0f 13 0d 78 56 34 12 	movlpd %xmm1,0x12345678
[ 	]*[a-f0-9]+:	66 0f 12 0d 78 56 34 12 	movlpd 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	0f 13 0d 78 56 34 12 	movlps %xmm1,0x12345678
[ 	]*[a-f0-9]+:	0f 12 0d 78 56 34 12 	movlps 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	f3 0f 16 0d 78 56 34 12 	movshdup 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	f3 0f 12 0d 78 56 34 12 	movsldup 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	f3 0f 70 0d 78 56 34 12 90 	pshufhw \$0x90,0x12345678,%xmm1
[ 	]*[a-f0-9]+:	f2 0f 70 0d 78 56 34 12 90 	pshuflw \$0x90,0x12345678,%xmm1
[ 	]*[a-f0-9]+:	0f 60 0d 78 56 34 12 	punpcklbw 0x12345678,%mm1
[ 	]*[a-f0-9]+:	0f 62 0d 78 56 34 12 	punpckldq 0x12345678,%mm1
[ 	]*[a-f0-9]+:	0f 61 0d 78 56 34 12 	punpcklwd 0x12345678,%mm1
[ 	]*[a-f0-9]+:	66 0f 60 0d 78 56 34 12 	punpcklbw 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	66 0f 62 0d 78 56 34 12 	punpckldq 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	66 0f 61 0d 78 56 34 12 	punpcklwd 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	66 0f 6c 0d 78 56 34 12 	punpcklqdq 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	66 0f 2e 0d 78 56 34 12 	ucomisd 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	0f 2e 0d 78 56 34 12 	ucomiss 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	f2 0f c2 00 00       	cmpeqsd \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f c2 00 00       	cmpeqss \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 2a 00          	cvtpi2pd \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	0f 2a 00             	cvtpi2ps \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	0f 2d 00             	cvtps2pi \(%eax\),%mm0
[ 	]*[a-f0-9]+:	f2 0f 2d 00          	cvtsd2si \(%eax\),%eax
[ 	]*[a-f0-9]+:	f2 0f 2c 00          	cvttsd2si \(%eax\),%eax
[ 	]*[a-f0-9]+:	f2 0f 5a 00          	cvtsd2ss \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 5a 00          	cvtss2sd \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 2d 00          	cvtss2si \(%eax\),%eax
[ 	]*[a-f0-9]+:	f3 0f 2c 00          	cvttss2si \(%eax\),%eax
[ 	]*[a-f0-9]+:	f2 0f 5e 00          	divsd  \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 5e 00          	divss  \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f2 0f 5f 00          	maxsd  \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 5f 00          	maxss  \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 5d 00          	minss  \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 5d 00          	minss  \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f2 0f 2b 00          	movntsd %xmm0,\(%eax\)
[ 	]*[a-f0-9]+:	f3 0f 2b 00          	movntss %xmm0,\(%eax\)
[ 	]*[a-f0-9]+:	f2 0f 10 00          	movsd  \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f2 0f 11 00          	movsd  %xmm0,\(%eax\)
[ 	]*[a-f0-9]+:	f3 0f 10 00          	movss  \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 11 00          	movss  %xmm0,\(%eax\)
[ 	]*[a-f0-9]+:	f2 0f 59 00          	mulsd  \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 59 00          	mulss  \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 53 00          	rcpss  \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 0b 00 00    	roundsd \$0x0,\(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 0a 00 00    	roundss \$0x0,\(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 52 00          	rsqrtss \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f2 0f 51 00          	sqrtsd \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 51 00          	sqrtss \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f2 0f 5c 00          	subsd  \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 5c 00          	subss  \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 20 00       	pmovsxbw \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 21 00       	pmovsxbd \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 22 00       	pmovsxbq \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 23 00       	pmovsxwd \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 24 00       	pmovsxwq \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 25 00       	pmovsxdq \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 30 00       	pmovzxbw \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 31 00       	pmovzxbd \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 32 00       	pmovzxbq \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 33 00       	pmovzxwd \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 34 00       	pmovzxwq \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 35 00       	pmovzxdq \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 21 00 00    	insertps \$0x0,\(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 15 08          	unpckhpd \(%eax\),%xmm1
[ 	]*[a-f0-9]+:	0f 15 08             	unpckhps \(%eax\),%xmm1
[ 	]*[a-f0-9]+:	66 0f 14 08          	unpcklpd \(%eax\),%xmm1
[ 	]*[a-f0-9]+:	0f 14 08             	unpcklps \(%eax\),%xmm1
[ 	]*[a-f0-9]+:	f3 0f c2 f7 10       	cmpss  \$0x10,%xmm7,%xmm6
[ 	]*[a-f0-9]+:	f3 0f c2 38 10       	cmpss  \$0x10,\(%eax\),%xmm7
[ 	]*[a-f0-9]+:	f2 0f c2 f7 10       	cmpsd  \$0x10,%xmm7,%xmm6
[ 	]*[a-f0-9]+:	f2 0f c2 38 10       	cmpsd  \$0x10,\(%eax\),%xmm7
[ 	]*[a-f0-9]+:	f3 0f 2a c8          	cvtsi2ss %eax,%xmm1
[ 	]*[a-f0-9]+:	f2 0f 2a c8          	cvtsi2sd %eax,%xmm1
[ 	]*[a-f0-9]+:	f3 0f 2a c8          	cvtsi2ss %eax,%xmm1
[ 	]*[a-f0-9]+:	f2 0f 2a c8          	cvtsi2sd %eax,%xmm1
[ 	]*[a-f0-9]+:	f3 0f 2a 08          	cvtsi2ss \(%eax\),%xmm1
[ 	]*[a-f0-9]+:	f2 0f 2a 08          	cvtsi2sd \(%eax\),%xmm1
[ 	]*[a-f0-9]+:	f3 0f 2a 08          	cvtsi2ss \(%eax\),%xmm1
[ 	]*[a-f0-9]+:	f2 0f 2a 08          	cvtsi2sd \(%eax\),%xmm1
[ 	]*[a-f0-9]+:	f2 0f d0 0d 78 56 34 12 	addsubps 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	66 0f 2f 0d 78 56 34 12 	comisd 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	0f 2f 0d 78 56 34 12 	comiss 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	f3 0f e6 0d 78 56 34 12 	cvtdq2pd 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	f2 0f e6 0d 78 56 34 12 	cvtpd2dq 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	0f 5a 0d 78 56 34 12 	cvtps2pd 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	f3 0f 5b 0d 78 56 34 12 	cvttps2dq 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	f2 0f 7c 0d 78 56 34 12 	haddps 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	f3 0f 7f 0d 78 56 34 12 	movdqu %xmm1,0x12345678
[ 	]*[a-f0-9]+:	f3 0f 6f 0d 78 56 34 12 	movdqu 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	66 0f 17 0d 78 56 34 12 	movhpd %xmm1,0x12345678
[ 	]*[a-f0-9]+:	66 0f 16 0d 78 56 34 12 	movhpd 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	0f 17 0d 78 56 34 12 	movhps %xmm1,0x12345678
[ 	]*[a-f0-9]+:	0f 16 0d 78 56 34 12 	movhps 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	66 0f 13 0d 78 56 34 12 	movlpd %xmm1,0x12345678
[ 	]*[a-f0-9]+:	66 0f 12 0d 78 56 34 12 	movlpd 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	0f 13 0d 78 56 34 12 	movlps %xmm1,0x12345678
[ 	]*[a-f0-9]+:	0f 12 0d 78 56 34 12 	movlps 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	f3 0f 16 0d 78 56 34 12 	movshdup 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	f3 0f 12 0d 78 56 34 12 	movsldup 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	f3 0f 70 0d 78 56 34 12 90 	pshufhw \$0x90,0x12345678,%xmm1
[ 	]*[a-f0-9]+:	f2 0f 70 0d 78 56 34 12 90 	pshuflw \$0x90,0x12345678,%xmm1
[ 	]*[a-f0-9]+:	0f 60 0d 78 56 34 12 	punpcklbw 0x12345678,%mm1
[ 	]*[a-f0-9]+:	0f 62 0d 78 56 34 12 	punpckldq 0x12345678,%mm1
[ 	]*[a-f0-9]+:	0f 61 0d 78 56 34 12 	punpcklwd 0x12345678,%mm1
[ 	]*[a-f0-9]+:	66 0f 60 0d 78 56 34 12 	punpcklbw 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	66 0f 62 0d 78 56 34 12 	punpckldq 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	66 0f 61 0d 78 56 34 12 	punpcklwd 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	66 0f 6c 0d 78 56 34 12 	punpcklqdq 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	66 0f 2e 0d 78 56 34 12 	ucomisd 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	0f 2e 0d 78 56 34 12 	ucomiss 0x12345678,%xmm1
[ 	]*[a-f0-9]+:	f2 0f c2 00 00       	cmpeqsd \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f c2 00 00       	cmpeqss \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 2a 00          	cvtpi2pd \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	0f 2a 00             	cvtpi2ps \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	0f 2d 00             	cvtps2pi \(%eax\),%mm0
[ 	]*[a-f0-9]+:	f2 0f 2d 00          	cvtsd2si \(%eax\),%eax
[ 	]*[a-f0-9]+:	f2 0f 2c 00          	cvttsd2si \(%eax\),%eax
[ 	]*[a-f0-9]+:	f2 0f 5a 00          	cvtsd2ss \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 5a 00          	cvtss2sd \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 2d 00          	cvtss2si \(%eax\),%eax
[ 	]*[a-f0-9]+:	f3 0f 2c 00          	cvttss2si \(%eax\),%eax
[ 	]*[a-f0-9]+:	f2 0f 5e 00          	divsd  \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 5e 00          	divss  \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f2 0f 5f 00          	maxsd  \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 5f 00          	maxss  \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 5d 00          	minss  \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 5d 00          	minss  \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f2 0f 2b 00          	movntsd %xmm0,\(%eax\)
[ 	]*[a-f0-9]+:	f3 0f 2b 00          	movntss %xmm0,\(%eax\)
[ 	]*[a-f0-9]+:	f2 0f 10 00          	movsd  \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f2 0f 11 00          	movsd  %xmm0,\(%eax\)
[ 	]*[a-f0-9]+:	f3 0f 10 00          	movss  \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 11 00          	movss  %xmm0,\(%eax\)
[ 	]*[a-f0-9]+:	f2 0f 59 00          	mulsd  \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 59 00          	mulss  \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 53 00          	rcpss  \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 0b 00 00    	roundsd \$0x0,\(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 0a 00 00    	roundss \$0x0,\(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 52 00          	rsqrtss \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f2 0f 51 00          	sqrtsd \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 51 00          	sqrtss \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f2 0f 5c 00          	subsd  \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 5c 00          	subss  \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 20 00       	pmovsxbw \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 21 00       	pmovsxbd \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 22 00       	pmovsxbq \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 23 00       	pmovsxwd \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 24 00       	pmovsxwq \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 25 00       	pmovsxdq \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 30 00       	pmovzxbw \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 31 00       	pmovzxbd \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 32 00       	pmovzxbq \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 33 00       	pmovzxwd \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 34 00       	pmovzxwq \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 35 00       	pmovzxdq \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 21 00 00    	insertps \$0x0,\(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 15 00          	unpckhpd \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	0f 15 00             	unpckhps \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 14 00          	unpcklpd \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	0f 14 00             	unpcklps \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f c2 f7 10       	cmpss  \$0x10,%xmm7,%xmm6
[ 	]*[a-f0-9]+:	f3 0f c2 38 10       	cmpss  \$0x10,\(%eax\),%xmm7
[ 	]*[a-f0-9]+:	f2 0f c2 f7 10       	cmpsd  \$0x10,%xmm7,%xmm6
[ 	]*[a-f0-9]+:	f2 0f c2 38 10       	cmpsd  \$0x10,\(%eax\),%xmm7
[ 	]*[a-f0-9]+:	f3 0f 2a c8          	cvtsi2ss %eax,%xmm1
[ 	]*[a-f0-9]+:	f2 0f 2a c8          	cvtsi2sd %eax,%xmm1
[ 	]*[a-f0-9]+:	f3 0f 2a c8          	cvtsi2ss %eax,%xmm1
[ 	]*[a-f0-9]+:	f2 0f 2a c8          	cvtsi2sd %eax,%xmm1
[ 	]*[a-f0-9]+:	f3 0f 2a 08          	cvtsi2ss \(%eax\),%xmm1
[ 	]*[a-f0-9]+:	f3 0f 2a 08          	cvtsi2ss \(%eax\),%xmm1
[ 	]*[a-f0-9]+:	f2 0f 2a 08          	cvtsi2sd \(%eax\),%xmm1
[ 	]*[a-f0-9]+:	f2 0f 2a 08          	cvtsi2sd \(%eax\),%xmm1
[ 	]*[a-f0-9]+:	f3 0f 2a 08          	cvtsi2ss \(%eax\),%xmm1
[ 	]*[a-f0-9]+:	f2 0f 2a 08          	cvtsi2sd \(%eax\),%xmm1
[ 	]*[a-f0-9]+:	0f 2c 00             	cvttps2pi \(%eax\),%mm0
#pass
