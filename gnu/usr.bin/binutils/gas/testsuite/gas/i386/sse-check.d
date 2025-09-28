#as: -msse-check=none
#objdump: -dw
#name: i386 SSE check (none)

.*:     file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	0f 58 ca             	addps  %xmm2,%xmm1
[ 	]*[a-f0-9]+:	66 0f 58 ca          	addpd  %xmm2,%xmm1
[ 	]*[a-f0-9]+:	66 0f 2a ca          	cvtpi2pd %mm2,%xmm1
[ 	]*[a-f0-9]+:	(67 )?66 0f 2a 0a(   )?       	cvtpi2pd \(%edx\),%xmm1
[ 	]*[a-f0-9]+:	66 0f d0 ca          	addsubpd %xmm2,%xmm1
[ 	]*[a-f0-9]+:	66 0f 38 01 ca       	phaddw %xmm2,%xmm1
[ 	]*[a-f0-9]+:	66 0f 38 15 c1       	blendvpd %xmm0,%xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 37 c1       	pcmpgtq %xmm1,%xmm0
[ 	]*[a-f0-9]+:	66 0f 78 c0 00 00    	extrq  \$0x0,\$0x0,%xmm0
[ 	]*[a-f0-9]+:	66 0f 3a 44 d1 ff    	pclmulqdq \$0xff,%xmm1,%xmm2
[ 	]*[a-f0-9]+:	66 0f 38 de d1       	aesdec %xmm1,%xmm2
[ 	]*[a-f0-9]+:	0f 38 c8 c0          	sha1nexte %xmm0,%xmm0
[ 	]*[a-f0-9]+:	66 0f 38 cf d1       	gf2p8mulb %xmm1,%xmm2
[ 	]*[a-f0-9]+:	62 f2 7d 09 cf c0    	vgf2p8mulb %xmm0,%xmm0,%xmm0\{%k1\}
[ 	]*[a-f0-9]+:	62 f2 7d 48 cf c0    	vgf2p8mulb %zmm0,%zmm0,%zmm0
#pass
