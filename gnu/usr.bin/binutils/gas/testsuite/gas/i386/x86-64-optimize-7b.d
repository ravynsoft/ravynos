#source: x86-64-optimize-7.s
#as: -O2 -march=+noavx512vl
#objdump: -drw
#name: x86-64 optimized encoding 7b with -O2

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	62 71 f5 4f 55 f9    	vandnpd %zmm1,%zmm1,%zmm15\{%k7\}
 +[a-f0-9]+:	c5 71 55 f9          	vandnpd %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	62 e1 f5 48 55 c1    	vandnpd %zmm1,%zmm1,%zmm16
 +[a-f0-9]+:	62 b1 f5 40 55 c9    	vandnpd %zmm17,%zmm17,%zmm1
 +[a-f0-9]+:	62 71 74 4f 55 f9    	vandnps %zmm1,%zmm1,%zmm15\{%k7\}
 +[a-f0-9]+:	c5 70 55 f9          	vandnps %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	62 e1 74 48 55 c1    	vandnps %zmm1,%zmm1,%zmm16
 +[a-f0-9]+:	62 b1 74 40 55 c9    	vandnps %zmm17,%zmm17,%zmm1
 +[a-f0-9]+:	62 71 75 4f df f9    	vpandnd %zmm1,%zmm1,%zmm15\{%k7\}
 +[a-f0-9]+:	c5 71 df f9          	vpandn %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	62 e1 75 48 df c1    	vpandnd %zmm1,%zmm1,%zmm16
 +[a-f0-9]+:	62 b1 75 40 df c9    	vpandnd %zmm17,%zmm17,%zmm1
 +[a-f0-9]+:	62 71 f5 4f df f9    	vpandnq %zmm1,%zmm1,%zmm15\{%k7\}
 +[a-f0-9]+:	c5 71 df f9          	vpandn %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	62 e1 f5 48 df c1    	vpandnq %zmm1,%zmm1,%zmm16
 +[a-f0-9]+:	62 b1 f5 40 df c9    	vpandnq %zmm17,%zmm17,%zmm1
 +[a-f0-9]+:	62 71 f5 4f 57 f9    	vxorpd %zmm1,%zmm1,%zmm15\{%k7\}
 +[a-f0-9]+:	c5 71 57 f9          	vxorpd %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	62 e1 f5 48 57 c1    	vxorpd %zmm1,%zmm1,%zmm16
 +[a-f0-9]+:	62 b1 f5 40 57 c9    	vxorpd %zmm17,%zmm17,%zmm1
 +[a-f0-9]+:	62 71 74 4f 57 f9    	vxorps %zmm1,%zmm1,%zmm15\{%k7\}
 +[a-f0-9]+:	c5 70 57 f9          	vxorps %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	62 e1 74 48 57 c1    	vxorps %zmm1,%zmm1,%zmm16
 +[a-f0-9]+:	62 b1 74 40 57 c9    	vxorps %zmm17,%zmm17,%zmm1
 +[a-f0-9]+:	62 71 75 4f ef f9    	vpxord %zmm1,%zmm1,%zmm15\{%k7\}
 +[a-f0-9]+:	c5 71 ef f9          	vpxor  %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	62 e1 75 48 ef c1    	vpxord %zmm1,%zmm1,%zmm16
 +[a-f0-9]+:	62 b1 75 40 ef c9    	vpxord %zmm17,%zmm17,%zmm1
 +[a-f0-9]+:	62 71 f5 4f ef f9    	vpxorq %zmm1,%zmm1,%zmm15\{%k7\}
 +[a-f0-9]+:	c5 71 ef f9          	vpxor  %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	62 e1 f5 48 ef c1    	vpxorq %zmm1,%zmm1,%zmm16
 +[a-f0-9]+:	62 b1 f5 40 ef c9    	vpxorq %zmm17,%zmm17,%zmm1
 +[a-f0-9]+:	62 71 75 4f f8 f9    	vpsubb %zmm1,%zmm1,%zmm15\{%k7\}
 +[a-f0-9]+:	c5 71 f8 f9          	vpsubb %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	62 e1 75 48 f8 c1    	vpsubb %zmm1,%zmm1,%zmm16
 +[a-f0-9]+:	62 b1 75 40 f8 c9    	vpsubb %zmm17,%zmm17,%zmm1
 +[a-f0-9]+:	62 71 75 4f f9 f9    	vpsubw %zmm1,%zmm1,%zmm15\{%k7\}
 +[a-f0-9]+:	c5 71 f9 f9          	vpsubw %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	62 e1 75 48 f9 c1    	vpsubw %zmm1,%zmm1,%zmm16
 +[a-f0-9]+:	62 b1 75 40 f9 c9    	vpsubw %zmm17,%zmm17,%zmm1
 +[a-f0-9]+:	62 71 75 4f fa f9    	vpsubd %zmm1,%zmm1,%zmm15\{%k7\}
 +[a-f0-9]+:	c5 71 fa f9          	vpsubd %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	62 e1 75 48 fa c1    	vpsubd %zmm1,%zmm1,%zmm16
 +[a-f0-9]+:	62 b1 75 40 fa c9    	vpsubd %zmm17,%zmm17,%zmm1
 +[a-f0-9]+:	62 71 f5 4f fb f9    	vpsubq %zmm1,%zmm1,%zmm15\{%k7\}
 +[a-f0-9]+:	c5 71 fb f9          	vpsubq %xmm1,%xmm1,%xmm15
 +[a-f0-9]+:	62 e1 f5 48 fb c1    	vpsubq %zmm1,%zmm1,%zmm16
 +[a-f0-9]+:	62 b1 f5 40 fb c9    	vpsubq %zmm17,%zmm17,%zmm1
#pass
