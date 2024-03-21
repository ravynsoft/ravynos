#as: -Os
#objdump: -drw
#name: x86-64 optimized encoding 4 with -Os

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	a9 7f 00 00 00       	test   \$0x7f,%eax
 +[a-f0-9]+:	c4 c1 61 db e4       	vpand  %xmm12,%xmm3,%xmm4
 +[a-f0-9]+:	c5 91 db e2          	vpand  %xmm2,%xmm13,%xmm4
 +[a-f0-9]+:	62 f1 7d 28 6f d1    	vmovdqa32 %ymm1,%ymm2
 +[a-f0-9]+:	62 f1 fd 28 6f d1    	vmovdqa64 %ymm1,%ymm2
 +[a-f0-9]+:	62 f1 7f 08 6f d1    	vmovdqu8 %xmm1,%xmm2
 +[a-f0-9]+:	62 f1 ff 08 6f d1    	vmovdqu16 %xmm1,%xmm2
 +[a-f0-9]+:	62 f1 7e 08 6f d1    	vmovdqu32 %xmm1,%xmm2
 +[a-f0-9]+:	62 f1 fe 08 6f d1    	vmovdqu64 %xmm1,%xmm2
 +[a-f0-9]+:	62 .*	vpandd %xmm2,%xmm3,%xmm4
 +[a-f0-9]+:	62 .*	vpandq %ymm12,%ymm3,%ymm4
 +[a-f0-9]+:	62 .*	vpandnd %ymm2,%ymm13,%ymm4
 +[a-f0-9]+:	62 .*	vpandnq %xmm2,%xmm3,%xmm14
 +[a-f0-9]+:	62 .*	vpord  %xmm2,%xmm3,%xmm4
 +[a-f0-9]+:	62 .*	vporq  %ymm12,%ymm3,%ymm4
 +[a-f0-9]+:	62 .*	vpxord %ymm2,%ymm13,%ymm4
 +[a-f0-9]+:	62 .*	vpxorq %xmm2,%xmm3,%xmm14
#pass
