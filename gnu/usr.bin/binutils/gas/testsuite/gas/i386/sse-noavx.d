#as: -msse-check=error
#objdump: -dw
#name: i386 SSE without AVX equivalent

.*:     file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	f2 0f 38 f0 d9       	crc32  %cl,%ebx
[ 	]*[a-f0-9]+:	66 0f 2d d3          	cvtpd2pi %xmm3,%mm2
[ 	]*[a-f0-9]+:	66 0f 2a d3          	cvtpi2pd %mm3,%xmm2
[ 	]*[a-f0-9]+:	0f 2a d3             	cvtpi2ps %mm3,%xmm2
[ 	]*[a-f0-9]+:	0f 2d f7             	cvtps2pi %xmm7,%mm6
[ 	]*[a-f0-9]+:	66 0f 2c dc          	cvttpd2pi %xmm4,%mm3
[ 	]*[a-f0-9]+:	0f 2c dc             	cvttps2pi %xmm4,%mm3
[ 	]*[a-f0-9]+:	df 08                	fisttps \(%eax\)
[ 	]*[a-f0-9]+:	db 08                	fisttpl \(%eax\)
[ 	]*[a-f0-9]+:	dd 08                	fisttpll \(%eax\)
[ 	]*[a-f0-9]+:	0f ae e8             	lfence
[ 	]*[a-f0-9]+:	0f f7 c7             	maskmovq %mm7,%mm0
[ 	]*[a-f0-9]+:	0f ae f0             	mfence
[ 	]*[a-f0-9]+:	0f 01 c8             	monitor %eax,%ecx,%edx
[ 	]*[a-f0-9]+:	f2 0f d6 c8          	movdq2q %xmm0,%mm1
[ 	]*[a-f0-9]+:	0f c3 00             	movnti %eax,\(%eax\)
[ 	]*[a-f0-9]+:	0f e7 10             	movntq %mm2,\(%eax\)
[ 	]*[a-f0-9]+:	f3 0f d6 c8          	movq2dq %mm0,%xmm1
[ 	]*[a-f0-9]+:	0f 01 c9             	mwait  %eax,%ecx
[ 	]*[a-f0-9]+:	0f 38 1c c1          	pabsb  %mm1,%mm0
[ 	]*[a-f0-9]+:	0f 38 1e c1          	pabsd  %mm1,%mm0
[ 	]*[a-f0-9]+:	0f 38 1d c1          	pabsw  %mm1,%mm0
[ 	]*[a-f0-9]+:	0f d4 c1             	paddq  %mm1,%mm0
[ 	]*[a-f0-9]+:	0f 3a 0f c1 02       	palignr \$0x2,%mm1,%mm0
[ 	]*[a-f0-9]+:	0f e0 c1             	pavgb  %mm1,%mm0
[ 	]*[a-f0-9]+:	0f e3 d3             	pavgw  %mm3,%mm2
[ 	]*[a-f0-9]+:	0f c5 c1 00          	pextrw \$0x0,%mm1,%eax
[ 	]*[a-f0-9]+:	0f 38 02 c1          	phaddd %mm1,%mm0
[ 	]*[a-f0-9]+:	0f 38 03 c1          	phaddsw %mm1,%mm0
[ 	]*[a-f0-9]+:	0f 38 01 c1          	phaddw %mm1,%mm0
[ 	]*[a-f0-9]+:	0f 38 06 c1          	phsubd %mm1,%mm0
[ 	]*[a-f0-9]+:	0f 38 07 c1          	phsubsw %mm1,%mm0
[ 	]*[a-f0-9]+:	0f 38 05 c1          	phsubw %mm1,%mm0
[ 	]*[a-f0-9]+:	0f c4 d2 02          	pinsrw \$0x2,%edx,%mm2
[ 	]*[a-f0-9]+:	0f 38 04 c1          	pmaddubsw %mm1,%mm0
[ 	]*[a-f0-9]+:	0f ee c1             	pmaxsw %mm1,%mm0
[ 	]*[a-f0-9]+:	0f de d2             	pmaxub %mm2,%mm2
[ 	]*[a-f0-9]+:	0f ea e5             	pminsw %mm5,%mm4
[ 	]*[a-f0-9]+:	0f da f7             	pminub %mm7,%mm6
[ 	]*[a-f0-9]+:	0f d7 c5             	pmovmskb %mm5,%eax
[ 	]*[a-f0-9]+:	0f 38 0b c1          	pmulhrsw %mm1,%mm0
[ 	]*[a-f0-9]+:	0f e4 e5             	pmulhuw %mm5,%mm4
[ 	]*[a-f0-9]+:	0f f4 c8             	pmuludq %mm0,%mm1
[ 	]*[a-f0-9]+:	f3 0f b8 cb          	popcnt %ebx,%ecx
[ 	]*[a-f0-9]+:	0f 18 00             	prefetchnta \(%eax\)
[ 	]*[a-f0-9]+:	0f 18 08             	prefetcht0 \(%eax\)
[ 	]*[a-f0-9]+:	0f 18 10             	prefetcht1 \(%eax\)
[ 	]*[a-f0-9]+:	0f 18 18             	prefetcht2 \(%eax\)
[ 	]*[a-f0-9]+:	0f f6 f7             	psadbw %mm7,%mm6
[ 	]*[a-f0-9]+:	0f 38 00 c1          	pshufb %mm1,%mm0
[ 	]*[a-f0-9]+:	0f 70 da 01          	pshufw \$0x1,%mm2,%mm3
[ 	]*[a-f0-9]+:	0f 38 08 c1          	psignb %mm1,%mm0
[ 	]*[a-f0-9]+:	0f 38 0a c1          	psignd %mm1,%mm0
[ 	]*[a-f0-9]+:	0f 38 09 c1          	psignw %mm1,%mm0
[ 	]*[a-f0-9]+:	0f fb c1             	psubq  %mm1,%mm0
[ 	]*[a-f0-9]+:	0f ae f8             	sfence
#pass
