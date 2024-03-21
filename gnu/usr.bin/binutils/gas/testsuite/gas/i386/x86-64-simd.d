#as: -J
#objdump: -dw
#name: x86-64 SIMD

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	f2 0f d0 0d 78 56 34 12 	addsubps 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	66 0f 2f 0d 78 56 34 12 	comisd 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	0f 2f 0d 78 56 34 12 	comiss 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	f3 0f e6 0d 78 56 34 12 	cvtdq2pd 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	f2 0f e6 0d 78 56 34 12 	cvtpd2dq 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	0f 5a 0d 78 56 34 12 	cvtps2pd 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	f3 0f 5b 0d 78 56 34 12 	cvttps2dq 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	f3 0f 2a c8          	cvtsi2ss %eax,%xmm1
[ 	]*[a-f0-9]+:	f2 0f 2a c8          	cvtsi2sd %eax,%xmm1
[ 	]*[a-f0-9]+:	f3 0f 2a c8          	cvtsi2ss %eax,%xmm1
[ 	]*[a-f0-9]+:	f2 0f 2a c8          	cvtsi2sd %eax,%xmm1
[ 	]*[a-f0-9]+:	f3 48 0f 2a c8       	cvtsi2ss %rax,%xmm1
[ 	]*[a-f0-9]+:	f2 48 0f 2a c8       	cvtsi2sd %rax,%xmm1
[ 	]*[a-f0-9]+:	f3 48 0f 2a c8       	cvtsi2ss %rax,%xmm1
[ 	]*[a-f0-9]+:	f2 48 0f 2a c8       	cvtsi2sd %rax,%xmm1
[ 	]*[a-f0-9]+:	f3 0f 2a 08          	cvtsi2ssl \(%rax\),%xmm1
[ 	]*[a-f0-9]+:	f2 0f 2a 08          	cvtsi2sdl \(%rax\),%xmm1
[ 	]*[a-f0-9]+:	f3 48 0f 2a 08       	cvtsi2ssq \(%rax\),%xmm1
[ 	]*[a-f0-9]+:	f2 48 0f 2a 08       	cvtsi2sdq \(%rax\),%xmm1
[ 	]*[a-f0-9]+:	f2 0f 7c 0d 78 56 34 12 	haddps 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	f3 0f 7f 0d 78 56 34 12 	movdqu %xmm1,0x12345678\(%rip\)        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	f3 0f 6f 0d 78 56 34 12 	movdqu 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	66 0f 17 0d 78 56 34 12 	movhpd %xmm1,0x12345678\(%rip\)        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	66 0f 16 0d 78 56 34 12 	movhpd 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	0f 17 0d 78 56 34 12 	movhps %xmm1,0x12345678\(%rip\)        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	0f 16 0d 78 56 34 12 	movhps 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	66 0f 13 0d 78 56 34 12 	movlpd %xmm1,0x12345678\(%rip\)        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	66 0f 12 0d 78 56 34 12 	movlpd 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	0f 13 0d 78 56 34 12 	movlps %xmm1,0x12345678\(%rip\)        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	0f 12 0d 78 56 34 12 	movlps 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	66 0f d6 0d 78 56 34 12 	movq   %xmm1,0x12345678\(%rip\)        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	f3 0f 7e 0d 78 56 34 12 	movq   0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	f3 0f 16 0d 78 56 34 12 	movshdup 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	f3 0f 12 0d 78 56 34 12 	movsldup 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	f3 0f 70 0d 78 56 34 12 90 	pshufhw \$0x90,0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	f2 0f 70 0d 78 56 34 12 90 	pshuflw \$0x90,0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	0f 60 0d 78 56 34 12 	punpcklbw 0x12345678\(%rip\),%mm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	0f 62 0d 78 56 34 12 	punpckldq 0x12345678\(%rip\),%mm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	0f 61 0d 78 56 34 12 	punpcklwd 0x12345678\(%rip\),%mm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	66 0f 60 0d 78 56 34 12 	punpcklbw 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	66 0f 62 0d 78 56 34 12 	punpckldq 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	66 0f 61 0d 78 56 34 12 	punpcklwd 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	66 0f 6c 0d 78 56 34 12 	punpcklqdq 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	66 0f 2e 0d 78 56 34 12 	ucomisd 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	0f 2e 0d 78 56 34 12 	ucomiss 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	f2 0f c2 00 00       	cmpeqsd \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f c2 00 00       	cmpeqss \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 2a 00          	cvtpi2pd \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	0f 2a 00             	cvtpi2ps \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	0f 2d 00             	cvtps2pi \(%rax\),%mm0
[ 	]*[a-f0-9]+:	f2 0f 2d 00          	cvtsd2si \(%rax\),%eax
[ 	]*[a-f0-9]+:	f2 48 0f 2d 00       	cvtsd2si \(%rax\),%rax
[ 	]*[a-f0-9]+:	f2 0f 2c 00          	cvttsd2si \(%rax\),%eax
[ 	]*[a-f0-9]+:	f2 48 0f 2c 00       	cvttsd2si \(%rax\),%rax
[ 	]*[a-f0-9]+:	f2 0f 5a 00          	cvtsd2ss \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 5a 00          	cvtss2sd \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 2d 00          	cvtss2si \(%rax\),%eax
[ 	]*[a-f0-9]+:	f3 48 0f 2d 00       	cvtss2si \(%rax\),%rax
[ 	]*[a-f0-9]+:	f3 0f 2c 00          	cvttss2si \(%rax\),%eax
[ 	]*[a-f0-9]+:	f3 48 0f 2c 00       	cvttss2si \(%rax\),%rax
[ 	]*[a-f0-9]+:	f2 0f 5e 00          	divsd  \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 5e 00          	divss  \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f2 0f 5f 00          	maxsd  \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 5f 00          	maxss  \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 5d 00          	minss  \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 5d 00          	minss  \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f2 0f 2b 00          	movntsd %xmm0,\(%rax\)
[ 	]*[a-f0-9]+:	f3 0f 2b 00          	movntss %xmm0,\(%rax\)
[ 	]*[a-f0-9]+:	f2 0f 10 00          	movsd  \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f2 0f 11 00          	movsd  %xmm0,\(%rax\)
[ 	]*[a-f0-9]+:	f3 0f 10 00          	movss  \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 11 00          	movss  %xmm0,\(%rax\)
[ 	]*[a-f0-9]+:	f2 0f 59 00          	mulsd  \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 59 00          	mulss  \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 53 00          	rcpss  \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 0b 00 00    	roundsd \$0x0,\(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 0a 00 00    	roundss \$0x0,\(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 52 00          	rsqrtss \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f2 0f 51 00          	sqrtsd \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 51 00          	sqrtss \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f2 0f 5c 00          	subsd  \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 5c 00          	subss  \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 20 00       	pmovsxbw \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 21 00       	pmovsxbd \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 22 00       	pmovsxbq \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 23 00       	pmovsxwd \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 24 00       	pmovsxwq \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 25 00       	pmovsxdq \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 30 00       	pmovzxbw \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 31 00       	pmovzxbd \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 32 00       	pmovzxbq \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 33 00       	pmovzxwd \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 34 00       	pmovzxwq \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 35 00       	pmovzxdq \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 21 00 00    	insertps \$0x0,\(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 15 08          	unpckhpd \(%rax\),%xmm1
[ 	]*[a-f0-9]+:	0f 15 08             	unpckhps \(%rax\),%xmm1
[ 	]*[a-f0-9]+:	66 0f 14 08          	unpcklpd \(%rax\),%xmm1
[ 	]*[a-f0-9]+:	0f 14 08             	unpcklps \(%rax\),%xmm1
[ 	]*[a-f0-9]+:	f3 0f c2 f7 10       	cmpss  \$0x10,%xmm7,%xmm6
[ 	]*[a-f0-9]+:	f3 0f c2 38 10       	cmpss  \$0x10,\(%rax\),%xmm7
[ 	]*[a-f0-9]+:	f2 0f c2 f7 10       	cmpsd  \$0x10,%xmm7,%xmm6
[ 	]*[a-f0-9]+:	f2 0f c2 38 10       	cmpsd  \$0x10,\(%rax\),%xmm7
[ 	]*[a-f0-9]+:	0f d4 c1             	paddq  %mm1,%mm0
[ 	]*[a-f0-9]+:	0f d4 00             	paddq  \(%rax\),%mm0
[ 	]*[a-f0-9]+:	66 0f d4 c1          	paddq  %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f d4 00          	paddq  \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	0f fb c1             	psubq  %mm1,%mm0
[ 	]*[a-f0-9]+:	0f fb 00             	psubq  \(%rax\),%mm0
[ 	]*[a-f0-9]+:	66 0f fb c1          	psubq  %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f fb 00          	psubq  \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	0f f4 c1             	pmuludq %mm1,%mm0
[ 	]*[a-f0-9]+:	0f f4 00             	pmuludq \(%rax\),%mm0
[ 	]*[a-f0-9]+:	66 0f f4 c1          	pmuludq %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f f4 00          	pmuludq \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f2 0f d0 0d 78 56 34 12 	addsubps 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	66 0f 2f 0d 78 56 34 12 	comisd 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	0f 2f 0d 78 56 34 12 	comiss 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	f3 0f e6 0d 78 56 34 12 	cvtdq2pd 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	f2 0f e6 0d 78 56 34 12 	cvtpd2dq 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	0f 5a 0d 78 56 34 12 	cvtps2pd 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	f3 0f 5b 0d 78 56 34 12 	cvttps2dq 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	f3 0f 2a c8          	cvtsi2ss %eax,%xmm1
[ 	]*[a-f0-9]+:	f2 0f 2a c8          	cvtsi2sd %eax,%xmm1
[ 	]*[a-f0-9]+:	f3 0f 2a c8          	cvtsi2ss %eax,%xmm1
[ 	]*[a-f0-9]+:	f2 0f 2a c8          	cvtsi2sd %eax,%xmm1
[ 	]*[a-f0-9]+:	f3 48 0f 2a c8       	cvtsi2ss %rax,%xmm1
[ 	]*[a-f0-9]+:	f2 48 0f 2a c8       	cvtsi2sd %rax,%xmm1
[ 	]*[a-f0-9]+:	f3 48 0f 2a c8       	cvtsi2ss %rax,%xmm1
[ 	]*[a-f0-9]+:	f2 48 0f 2a c8       	cvtsi2sd %rax,%xmm1
[ 	]*[a-f0-9]+:	f3 0f 2a 08          	cvtsi2ssl \(%rax\),%xmm1
[ 	]*[a-f0-9]+:	f2 0f 2a 08          	cvtsi2sdl \(%rax\),%xmm1
[ 	]*[a-f0-9]+:	f3 0f 2a 08          	cvtsi2ssl \(%rax\),%xmm1
[ 	]*[a-f0-9]+:	f2 0f 2a 08          	cvtsi2sdl \(%rax\),%xmm1
[ 	]*[a-f0-9]+:	f3 48 0f 2a 08       	cvtsi2ssq \(%rax\),%xmm1
[ 	]*[a-f0-9]+:	f2 48 0f 2a 08       	cvtsi2sdq \(%rax\),%xmm1
[ 	]*[a-f0-9]+:	f3 48 0f 2a 08       	cvtsi2ssq \(%rax\),%xmm1
[ 	]*[a-f0-9]+:	f2 48 0f 2a 08       	cvtsi2sdq \(%rax\),%xmm1
[ 	]*[a-f0-9]+:	f2 0f 7c 0d 78 56 34 12 	haddps 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	f3 0f 7f 0d 78 56 34 12 	movdqu %xmm1,0x12345678\(%rip\)        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	f3 0f 6f 0d 78 56 34 12 	movdqu 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	66 0f 17 0d 78 56 34 12 	movhpd %xmm1,0x12345678\(%rip\)        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	66 0f 16 0d 78 56 34 12 	movhpd 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	0f 17 0d 78 56 34 12 	movhps %xmm1,0x12345678\(%rip\)        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	0f 16 0d 78 56 34 12 	movhps 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	66 0f 13 0d 78 56 34 12 	movlpd %xmm1,0x12345678\(%rip\)        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	66 0f 12 0d 78 56 34 12 	movlpd 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	0f 13 0d 78 56 34 12 	movlps %xmm1,0x12345678\(%rip\)        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	0f 12 0d 78 56 34 12 	movlps 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	66 0f d6 0d 78 56 34 12 	movq   %xmm1,0x12345678\(%rip\)        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	f3 0f 7e 0d 78 56 34 12 	movq   0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	f3 0f 16 0d 78 56 34 12 	movshdup 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	f3 0f 12 0d 78 56 34 12 	movsldup 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	f3 0f 70 0d 78 56 34 12 90 	pshufhw \$0x90,0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	f2 0f 70 0d 78 56 34 12 90 	pshuflw \$0x90,0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	0f 60 0d 78 56 34 12 	punpcklbw 0x12345678\(%rip\),%mm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	0f 62 0d 78 56 34 12 	punpckldq 0x12345678\(%rip\),%mm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	0f 61 0d 78 56 34 12 	punpcklwd 0x12345678\(%rip\),%mm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	66 0f 60 0d 78 56 34 12 	punpcklbw 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	66 0f 62 0d 78 56 34 12 	punpckldq 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	66 0f 61 0d 78 56 34 12 	punpcklwd 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	66 0f 6c 0d 78 56 34 12 	punpcklqdq 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	66 0f 2e 0d 78 56 34 12 	ucomisd 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	0f 2e 0d 78 56 34 12 	ucomiss 0x12345678\(%rip\),%xmm1        # [0-9a-f]+ <_start\+0x[0-9a-f]+>
[ 	]*[a-f0-9]+:	f2 0f c2 00 00       	cmpeqsd \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f c2 00 00       	cmpeqss \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 2a 00          	cvtpi2pd \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	0f 2a 00             	cvtpi2ps \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	0f 2d 00             	cvtps2pi \(%rax\),%mm0
[ 	]*[a-f0-9]+:	f2 0f 2d 00          	cvtsd2si \(%rax\),%eax
[ 	]*[a-f0-9]+:	f2 48 0f 2d 00       	cvtsd2si \(%rax\),%rax
[ 	]*[a-f0-9]+:	f2 0f 2c 00          	cvttsd2si \(%rax\),%eax
[ 	]*[a-f0-9]+:	f2 48 0f 2c 00       	cvttsd2si \(%rax\),%rax
[ 	]*[a-f0-9]+:	f2 0f 5a 00          	cvtsd2ss \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 5a 00          	cvtss2sd \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 2d 00          	cvtss2si \(%rax\),%eax
[ 	]*[a-f0-9]+:	f3 48 0f 2d 00       	cvtss2si \(%rax\),%rax
[ 	]*[a-f0-9]+:	f3 0f 2c 00          	cvttss2si \(%rax\),%eax
[ 	]*[a-f0-9]+:	f3 48 0f 2c 00       	cvttss2si \(%rax\),%rax
[ 	]*[a-f0-9]+:	f2 0f 5e 00          	divsd  \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 5e 00          	divss  \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f2 0f 5f 00          	maxsd  \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 5f 00          	maxss  \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 5d 00          	minss  \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 5d 00          	minss  \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f2 0f 2b 00          	movntsd %xmm0,\(%rax\)
[ 	]*[a-f0-9]+:	f3 0f 2b 00          	movntss %xmm0,\(%rax\)
[ 	]*[a-f0-9]+:	f2 0f 10 00          	movsd  \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f2 0f 11 00          	movsd  %xmm0,\(%rax\)
[ 	]*[a-f0-9]+:	f3 0f 10 00          	movss  \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 11 00          	movss  %xmm0,\(%rax\)
[ 	]*[a-f0-9]+:	f2 0f 59 00          	mulsd  \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 59 00          	mulss  \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 53 00          	rcpss  \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 0b 00 00    	roundsd \$0x0,\(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 0a 00 00    	roundss \$0x0,\(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 52 00          	rsqrtss \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f2 0f 51 00          	sqrtsd \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 51 00          	sqrtss \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f2 0f 5c 00          	subsd  \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f 5c 00          	subss  \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 20 00       	pmovsxbw \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 21 00       	pmovsxbd \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 22 00       	pmovsxbq \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 23 00       	pmovsxwd \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 24 00       	pmovsxwq \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 25 00       	pmovsxdq \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 30 00       	pmovzxbw \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 31 00       	pmovzxbd \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 32 00       	pmovzxbq \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 33 00       	pmovzxwd \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 34 00       	pmovzxwq \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 35 00       	pmovzxdq \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 21 00 00    	insertps \$0x0,\(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 15 00          	unpckhpd \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	0f 15 00             	unpckhps \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 14 00          	unpcklpd \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	0f 14 00             	unpcklps \(%rax\),%xmm0
[ 	]*[a-f0-9]+:	f3 0f c2 f7 10       	cmpss  \$0x10,%xmm7,%xmm6
[ 	]*[a-f0-9]+:	f3 0f c2 38 10       	cmpss  \$0x10,\(%rax\),%xmm7
[ 	]*[a-f0-9]+:	f2 0f c2 f7 10       	cmpsd  \$0x10,%xmm7,%xmm6
[ 	]*[a-f0-9]+:	f2 0f c2 38 10       	cmpsd  \$0x10,\(%rax\),%xmm7
[ 	]*[a-f0-9]+:	0f d4 08             	paddq  \(%rax\),%mm1
[ 	]*[a-f0-9]+:	0f d4 08             	paddq  \(%rax\),%mm1
[ 	]*[a-f0-9]+:	66 0f d4 08          	paddq  \(%rax\),%xmm1
[ 	]*[a-f0-9]+:	66 0f d4 08          	paddq  \(%rax\),%xmm1
[ 	]*[a-f0-9]+:	0f fb 08             	psubq  \(%rax\),%mm1
[ 	]*[a-f0-9]+:	0f fb 08             	psubq  \(%rax\),%mm1
[ 	]*[a-f0-9]+:	66 0f fb 08          	psubq  \(%rax\),%xmm1
[ 	]*[a-f0-9]+:	66 0f fb 08          	psubq  \(%rax\),%xmm1
[ 	]*[a-f0-9]+:	0f f4 08             	pmuludq \(%rax\),%mm1
[ 	]*[a-f0-9]+:	0f f4 08             	pmuludq \(%rax\),%mm1
[ 	]*[a-f0-9]+:	66 0f f4 08          	pmuludq \(%rax\),%xmm1
[ 	]*[a-f0-9]+:	66 0f f4 08          	pmuludq \(%rax\),%xmm1
#pass
