#as: -I${srcdir}/$subdir
#objdump: -dwMaddr16 -Mdata16
#name: i386 16-bit SSE2

.*:     file format .*

Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	67 0f c3 00          	movnti %eax,\(%eax\)
[ 	]*[a-f0-9]+:	0f ae f8             	sfence
[ 	]*[a-f0-9]+:	0f ae e8             	lfence
[ 	]*[a-f0-9]+:	0f ae f0             	mfence
[ 	]*[a-f0-9]+:	67 66 0f 58 01       	addpd  \(%ecx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 58 ca          	addpd  %xmm2,%xmm1
[ 	]*[a-f0-9]+:	67 f2 0f 58 13       	addsd  \(%ebx\),%xmm2
[ 	]*[a-f0-9]+:	f2 0f 58 dc          	addsd  %xmm4,%xmm3
[ 	]*[a-f0-9]+:	67 66 0f 55 65 00    	andnpd 0x0\(%ebp\),%xmm4
[ 	]*[a-f0-9]+:	66 0f 55 ee          	andnpd %xmm6,%xmm5
[ 	]*[a-f0-9]+:	67 66 0f 54 37       	andpd  \(%edi\),%xmm6
[ 	]*[a-f0-9]+:	66 0f 54 f8          	andpd  %xmm0,%xmm7
[ 	]*[a-f0-9]+:	66 0f c2 c1 02       	cmplepd %xmm1,%xmm0
[ 	]*[a-f0-9]+:	67 66 0f c2 0a 03    	cmpunordpd \(%edx\),%xmm1
[ 	]*[a-f0-9]+:	f2 0f c2 d2 04       	cmpneqsd %xmm2,%xmm2
[ 	]*[a-f0-9]+:	67 f2 0f c2 1c 24 05 	cmpnltsd \(%esp\),%xmm3
[ 	]*[a-f0-9]+:	66 0f c2 e5 06       	cmpnlepd %xmm5,%xmm4
[ 	]*[a-f0-9]+:	67 66 0f c2 2e 07    	cmpordpd \(%esi\),%xmm5
[ 	]*[a-f0-9]+:	f2 0f c2 f7 00       	cmpeqsd %xmm7,%xmm6
[ 	]*[a-f0-9]+:	67 f2 0f c2 38 01    	cmpltsd \(%eax\),%xmm7
[ 	]*[a-f0-9]+:	66 0f c2 c1 00       	cmpeqpd %xmm1,%xmm0
[ 	]*[a-f0-9]+:	67 66 0f c2 0a 00    	cmpeqpd \(%edx\),%xmm1
[ 	]*[a-f0-9]+:	f2 0f c2 d2 00       	cmpeqsd %xmm2,%xmm2
[ 	]*[a-f0-9]+:	67 f2 0f c2 1c 24 00 	cmpeqsd \(%esp\),%xmm3
[ 	]*[a-f0-9]+:	66 0f c2 e5 01       	cmpltpd %xmm5,%xmm4
[ 	]*[a-f0-9]+:	67 66 0f c2 2e 01    	cmpltpd \(%esi\),%xmm5
[ 	]*[a-f0-9]+:	f2 0f c2 f7 01       	cmpltsd %xmm7,%xmm6
[ 	]*[a-f0-9]+:	67 f2 0f c2 38 01    	cmpltsd \(%eax\),%xmm7
[ 	]*[a-f0-9]+:	67 66 0f c2 01 02    	cmplepd \(%ecx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f c2 ca 02       	cmplepd %xmm2,%xmm1
[ 	]*[a-f0-9]+:	67 f2 0f c2 13 02    	cmplesd \(%ebx\),%xmm2
[ 	]*[a-f0-9]+:	f2 0f c2 dc 02       	cmplesd %xmm4,%xmm3
[ 	]*[a-f0-9]+:	67 66 0f c2 65 00 03 	cmpunordpd 0x0\(%ebp\),%xmm4
[ 	]*[a-f0-9]+:	66 0f c2 ee 03       	cmpunordpd %xmm6,%xmm5
[ 	]*[a-f0-9]+:	67 f2 0f c2 37 03    	cmpunordsd \(%edi\),%xmm6
[ 	]*[a-f0-9]+:	f2 0f c2 f8 03       	cmpunordsd %xmm0,%xmm7
[ 	]*[a-f0-9]+:	66 0f c2 c1 04       	cmpneqpd %xmm1,%xmm0
[ 	]*[a-f0-9]+:	67 66 0f c2 0a 04    	cmpneqpd \(%edx\),%xmm1
[ 	]*[a-f0-9]+:	f2 0f c2 d2 04       	cmpneqsd %xmm2,%xmm2
[ 	]*[a-f0-9]+:	67 f2 0f c2 1c 24 04 	cmpneqsd \(%esp\),%xmm3
[ 	]*[a-f0-9]+:	66 0f c2 e5 05       	cmpnltpd %xmm5,%xmm4
[ 	]*[a-f0-9]+:	67 66 0f c2 2e 05    	cmpnltpd \(%esi\),%xmm5
[ 	]*[a-f0-9]+:	f2 0f c2 f7 05       	cmpnltsd %xmm7,%xmm6
[ 	]*[a-f0-9]+:	67 f2 0f c2 38 05    	cmpnltsd \(%eax\),%xmm7
[ 	]*[a-f0-9]+:	67 66 0f c2 01 06    	cmpnlepd \(%ecx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f c2 ca 06       	cmpnlepd %xmm2,%xmm1
[ 	]*[a-f0-9]+:	67 f2 0f c2 13 06    	cmpnlesd \(%ebx\),%xmm2
[ 	]*[a-f0-9]+:	f2 0f c2 dc 06       	cmpnlesd %xmm4,%xmm3
[ 	]*[a-f0-9]+:	67 66 0f c2 65 00 07 	cmpordpd 0x0\(%ebp\),%xmm4
[ 	]*[a-f0-9]+:	66 0f c2 ee 07       	cmpordpd %xmm6,%xmm5
[ 	]*[a-f0-9]+:	67 f2 0f c2 37 07    	cmpordsd \(%edi\),%xmm6
[ 	]*[a-f0-9]+:	f2 0f c2 f8 07       	cmpordsd %xmm0,%xmm7
[ 	]*[a-f0-9]+:	66 0f 2f c1          	comisd %xmm1,%xmm0
[ 	]*[a-f0-9]+:	67 66 0f 2f 0a       	comisd \(%edx\),%xmm1
[ 	]*[a-f0-9]+:	66 0f 2a d3          	cvtpi2pd %mm3,%xmm2
[ 	]*[a-f0-9]+:	67 66 0f 2a 1c 24    	cvtpi2pd \(%esp\),%xmm3
[ 	]*[a-f0-9]+:	f2 0f 2a e5          	cvtsi2sd %ebp,%xmm4
[ 	]*[a-f0-9]+:	67 f2 0f 2a 2e       	cvtsi2sd \(%esi\),%xmm5
[ 	]*[a-f0-9]+:	66 0f 2d f7          	cvtpd2pi %xmm7,%mm6
[ 	]*[a-f0-9]+:	67 66 0f 2d 38       	cvtpd2pi \(%eax\),%mm7
[ 	]*[a-f0-9]+:	67 f2 0f 2d 01       	cvtsd2si \(%ecx\),%eax
[ 	]*[a-f0-9]+:	f2 0f 2d ca          	cvtsd2si %xmm2,%ecx
[ 	]*[a-f0-9]+:	67 66 0f 2c 13       	cvttpd2pi \(%ebx\),%mm2
[ 	]*[a-f0-9]+:	66 0f 2c dc          	cvttpd2pi %xmm4,%mm3
[ 	]*[a-f0-9]+:	67 f2 0f 2c 65 00    	cvttsd2si 0x0\(%ebp\),%esp
[ 	]*[a-f0-9]+:	f2 0f 2c ee          	cvttsd2si %xmm6,%ebp
[ 	]*[a-f0-9]+:	66 0f 5e c1          	divpd  %xmm1,%xmm0
[ 	]*[a-f0-9]+:	67 66 0f 5e 0a       	divpd  \(%edx\),%xmm1
[ 	]*[a-f0-9]+:	f2 0f 5e d3          	divsd  %xmm3,%xmm2
[ 	]*[a-f0-9]+:	67 f2 0f 5e 1c 24    	divsd  \(%esp\),%xmm3
[ 	]*[a-f0-9]+:	67 0f ae 55 00       	ldmxcsr 0x0\(%ebp\)
[ 	]*[a-f0-9]+:	67 0f ae 1e          	stmxcsr \(%esi\)
[ 	]*[a-f0-9]+:	0f ae f8             	sfence
[ 	]*[a-f0-9]+:	66 0f 5f c1          	maxpd  %xmm1,%xmm0
[ 	]*[a-f0-9]+:	67 66 0f 5f 0a       	maxpd  \(%edx\),%xmm1
[ 	]*[a-f0-9]+:	f2 0f 5f d3          	maxsd  %xmm3,%xmm2
[ 	]*[a-f0-9]+:	67 f2 0f 5f 1c 24    	maxsd  \(%esp\),%xmm3
[ 	]*[a-f0-9]+:	66 0f 5d e5          	minpd  %xmm5,%xmm4
[ 	]*[a-f0-9]+:	67 66 0f 5d 2e       	minpd  \(%esi\),%xmm5
[ 	]*[a-f0-9]+:	f2 0f 5d f7          	minsd  %xmm7,%xmm6
[ 	]*[a-f0-9]+:	67 f2 0f 5d 38       	minsd  \(%eax\),%xmm7
[ 	]*[a-f0-9]+:	66 0f 28 c1          	movapd %xmm1,%xmm0
[ 	]*[a-f0-9]+:	67 66 0f 29 11       	movapd %xmm2,\(%ecx\)
[ 	]*[a-f0-9]+:	67 66 0f 28 12       	movapd \(%edx\),%xmm2
[ 	]*[a-f0-9]+:	67 66 0f 17 2c 24    	movhpd %xmm5,\(%esp\)
[ 	]*[a-f0-9]+:	67 66 0f 16 2e       	movhpd \(%esi\),%xmm5
[ 	]*[a-f0-9]+:	67 66 0f 13 07       	movlpd %xmm0,\(%edi\)
[ 	]*[a-f0-9]+:	67 66 0f 12 00       	movlpd \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 50 ca          	movmskpd %xmm2,%ecx
[ 	]*[a-f0-9]+:	66 0f 10 d3          	movupd %xmm3,%xmm2
[ 	]*[a-f0-9]+:	67 66 0f 11 22       	movupd %xmm4,\(%edx\)
[ 	]*[a-f0-9]+:	67 66 0f 10 65 00    	movupd 0x0\(%ebp\),%xmm4
[ 	]*[a-f0-9]+:	f2 0f 10 ee          	movsd  %xmm6,%xmm5
[ 	]*[a-f0-9]+:	67 f2 0f 11 3e       	movsd  %xmm7,\(%esi\)
[ 	]*[a-f0-9]+:	67 f2 0f 10 38       	movsd  \(%eax\),%xmm7
[ 	]*[a-f0-9]+:	66 0f 59 c1          	mulpd  %xmm1,%xmm0
[ 	]*[a-f0-9]+:	67 66 0f 59 0a       	mulpd  \(%edx\),%xmm1
[ 	]*[a-f0-9]+:	f2 0f 59 d2          	mulsd  %xmm2,%xmm2
[ 	]*[a-f0-9]+:	67 f2 0f 59 1c 24    	mulsd  \(%esp\),%xmm3
[ 	]*[a-f0-9]+:	66 0f 56 e5          	orpd   %xmm5,%xmm4
[ 	]*[a-f0-9]+:	67 66 0f 56 2e       	orpd   \(%esi\),%xmm5
[ 	]*[a-f0-9]+:	67 66 0f c6 37 02    	shufpd \$0x2,\(%edi\),%xmm6
[ 	]*[a-f0-9]+:	66 0f c6 f8 03       	shufpd \$0x3,%xmm0,%xmm7
[ 	]*[a-f0-9]+:	66 0f 51 c1          	sqrtpd %xmm1,%xmm0
[ 	]*[a-f0-9]+:	67 66 0f 51 0a       	sqrtpd \(%edx\),%xmm1
[ 	]*[a-f0-9]+:	f2 0f 51 d2          	sqrtsd %xmm2,%xmm2
[ 	]*[a-f0-9]+:	67 f2 0f 51 1c 24    	sqrtsd \(%esp\),%xmm3
[ 	]*[a-f0-9]+:	66 0f 5c e5          	subpd  %xmm5,%xmm4
[ 	]*[a-f0-9]+:	67 66 0f 5c 2e       	subpd  \(%esi\),%xmm5
[ 	]*[a-f0-9]+:	f2 0f 5c f7          	subsd  %xmm7,%xmm6
[ 	]*[a-f0-9]+:	67 f2 0f 5c 38       	subsd  \(%eax\),%xmm7
[ 	]*[a-f0-9]+:	67 66 0f 2e 01       	ucomisd \(%ecx\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 2e ca          	ucomisd %xmm2,%xmm1
[ 	]*[a-f0-9]+:	67 66 0f 15 13       	unpckhpd \(%ebx\),%xmm2
[ 	]*[a-f0-9]+:	66 0f 15 dc          	unpckhpd %xmm4,%xmm3
[ 	]*[a-f0-9]+:	67 66 0f 14 65 00    	unpcklpd 0x0\(%ebp\),%xmm4
[ 	]*[a-f0-9]+:	66 0f 14 ee          	unpcklpd %xmm6,%xmm5
[ 	]*[a-f0-9]+:	67 66 0f 57 37       	xorpd  \(%edi\),%xmm6
[ 	]*[a-f0-9]+:	66 0f 57 f8          	xorpd  %xmm0,%xmm7
[ 	]*[a-f0-9]+:	67 66 0f 2b 33       	movntpd %xmm6,\(%ebx\)
[ 	]*[a-f0-9]+:	66 0f 57 c8          	xorpd  %xmm0,%xmm1
[ 	]*[a-f0-9]+:	f3 0f e6 c8          	cvtdq2pd %xmm0,%xmm1
[ 	]*[a-f0-9]+:	f2 0f e6 c8          	cvtpd2dq %xmm0,%xmm1
[ 	]*[a-f0-9]+:	0f 5b c8             	cvtdq2ps %xmm0,%xmm1
[ 	]*[a-f0-9]+:	66 0f 5a c8          	cvtpd2ps %xmm0,%xmm1
[ 	]*[a-f0-9]+:	0f 5a c8             	cvtps2pd %xmm0,%xmm1
[ 	]*[a-f0-9]+:	66 0f 5b c8          	cvtps2dq %xmm0,%xmm1
[ 	]*[a-f0-9]+:	f2 0f 5a c8          	cvtsd2ss %xmm0,%xmm1
[ 	]*[a-f0-9]+:	f3 0f 5a c8          	cvtss2sd %xmm0,%xmm1
[ 	]*[a-f0-9]+:	66 0f e6 c8          	cvttpd2dq %xmm0,%xmm1
[ 	]*[a-f0-9]+:	f3 0f 5b c8          	cvttps2dq %xmm0,%xmm1
[ 	]*[a-f0-9]+:	66 0f f7 c8          	maskmovdqu %xmm0,%xmm1
[ 	]*[a-f0-9]+:	66 0f 6f c8          	movdqa %xmm0,%xmm1
[ 	]*[a-f0-9]+:	67 66 0f 7f 06       	movdqa %xmm0,\(%esi\)
[ 	]*[a-f0-9]+:	f3 0f 6f c8          	movdqu %xmm0,%xmm1
[ 	]*[a-f0-9]+:	67 f3 0f 7f 06       	movdqu %xmm0,\(%esi\)
[ 	]*[a-f0-9]+:	f2 0f d6 c8          	movdq2q %xmm0,%mm1
[ 	]*[a-f0-9]+:	f3 0f d6 c8          	movq2dq %mm0,%xmm1
[ 	]*[a-f0-9]+:	0f f4 c8             	pmuludq %mm0,%mm1
[ 	]*[a-f0-9]+:	67 0f f4 08          	pmuludq \(%eax\),%mm1
[ 	]*[a-f0-9]+:	66 0f f4 c8          	pmuludq %xmm0,%xmm1
[ 	]*[a-f0-9]+:	67 66 0f f4 08       	pmuludq \(%eax\),%xmm1
[ 	]*[a-f0-9]+:	66 0f 70 c8 01       	pshufd \$0x1,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	f3 0f 70 c8 01       	pshufhw \$0x1,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	f2 0f 70 c8 01       	pshuflw \$0x1,%xmm0,%xmm1
[ 	]*[a-f0-9]+:	66 0f 73 f8 01       	pslldq \$0x1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 73 d8 01       	psrldq \$0x1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 6d c8          	punpckhqdq %xmm0,%xmm1
[ 	]*[a-f0-9]+:	0f d4 c1             	paddq  %mm1,%mm0
[ 	]*[a-f0-9]+:	67 0f d4 00          	paddq  \(%eax\),%mm0
[ 	]*[a-f0-9]+:	66 0f d4 c1          	paddq  %xmm1,%xmm0
[ 	]*[a-f0-9]+:	67 66 0f d4 00       	paddq  \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	0f fb c1             	psubq  %mm1,%mm0
[ 	]*[a-f0-9]+:	67 0f fb 00          	psubq  \(%eax\),%mm0
[ 	]*[a-f0-9]+:	66 0f fb c1          	psubq  %xmm1,%xmm0
[ 	]*[a-f0-9]+:	67 66 0f fb 00       	psubq  \(%eax\),%xmm0
[ 	]*[a-f0-9]+:	0f 58 2f             	addps  \(%bx\),%xmm5
[ 	]*[a-f0-9]+:	f3 0f 2a d9          	cvtsi2ss %ecx,%xmm3
[ 	]*[a-f0-9]+:	f3 0f 2d cb          	cvtss2si %xmm3,%ecx
[ 	]*[a-f0-9]+:	f3 0f 2c cb          	cvttss2si %xmm3,%ecx
[ 	]*[a-f0-9]+:	66 0f 3a 17 ca 00    	extractps \$0x0,%xmm1,%edx
[ 	]*[a-f0-9]+:	0f 50 ca             	movmskps %xmm2,%ecx
[ 	]*[a-f0-9]+:	66 0f 3a 14 ca 00    	pextrb \$0x0,%xmm1,%edx
[ 	]*[a-f0-9]+:	66 0f 3a 16 ca 00    	pextrd \$0x0,%xmm1,%edx
[ 	]*[a-f0-9]+:	0f c5 d1 00          	pextrw \$0x0,%mm1,%edx
[ 	]*[a-f0-9]+:	66 0f c5 d1 00       	pextrw \$0x0,%xmm1,%edx
[ 	]*[a-f0-9]+:	66 0f 3a 20 d1 00    	pinsrb \$0x0,%ecx,%xmm2
[ 	]*[a-f0-9]+:	66 0f 3a 22 d1 00    	pinsrd \$0x0,%ecx,%xmm2
[ 	]*[a-f0-9]+:	0f c4 d1 00          	pinsrw \$0x0,%ecx,%mm2
[ 	]*[a-f0-9]+:	66 0f c4 d1 00       	pinsrw \$0x0,%ecx,%xmm2
[ 	]*[a-f0-9]+:	66 0f d7 d3          	pmovmskb %xmm3,%edx
[ 	]*[a-f0-9]+:	f3 0f 2a 05          	cvtsi2ss \(%di\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 17 0d 00    	extractps \$0x0,%xmm1,\(%di\)
[ 	]*[a-f0-9]+:	66 0f 3a 21 05 00    	insertps \$0x0,\(%di\),%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 16 0d 00    	pextrd \$0x0,%xmm1,\(%di\)
[ 	]*[a-f0-9]+:	66 0f 3a 22 05 00    	pinsrd \$0x0,\(%di\),%xmm0
#pass
